#!/usr/bin/env python3

from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


BACKEND_EMIT_OBJ_FLAG = "--emit-obj"
BACKEND_CONVERT_OBJ_FLAG = "--convert-splobj"  # change to "--convert-obj" if your backend uses that name


def fail(message: str) -> None:
    print(f"splc: error: {message}", file=sys.stderr)
    raise SystemExit(1)


def run(cmd: list[str], *, verbose: bool) -> None:
    if verbose:
        print("+ " + " ".join(cmd), file=sys.stderr)

    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError as error:
        fail(f"command failed with exit code {error.returncode}: {' '.join(cmd)}")


def require_executable(path: Path) -> Path:
    if not path.exists():
        fail(f"tool not found: {path}")
    if not path.is_file():
        fail(f"tool is not a file: {path}")
    if not path.stat().st_mode & 0o111:
        fail(f"tool is not executable: {path}")
    return path


def default_output_for(inputs: list[Path], emit_obj: bool) -> Path:
    if len(inputs) == 1:
        src = inputs[0]
        return src.with_suffix(".splobj" if emit_obj else ".out")

    return Path("a.splobj" if emit_obj else "a.out")


def replace_suffix(path: Path, suffix: str) -> Path:
    return path.with_suffix(suffix)


class Toolchain:
    def __init__(self, bin_dir: Path) -> None:
        self.frontend = require_executable(bin_dir / "frontend")
        self.midend = require_executable(bin_dir / "midend")
        self.backend = require_executable(bin_dir / "backend")
        self.spl_link = require_executable(bin_dir / "spl_link")

    def compile_to_middle(self, src: Path, front: Path, middle: Path, *, verbose: bool) -> None:
        front.parent.mkdir(parents=True, exist_ok=True)
        middle.parent.mkdir(parents=True, exist_ok=True)

        run([str(self.frontend), str(src), str(front)], verbose=verbose)
        run([str(self.midend), "-i", str(front), "-o", str(middle)], verbose=verbose)

    def compile_to_executable(
        self,
        src: Path,
        out: Path,
        asm: Path | None,
        *,
        verbose: bool,
    ) -> None:
        front = replace_suffix(out, ".front")
        middle = replace_suffix(out, ".middle")

        self.compile_to_middle(src, front, middle, verbose=verbose)

        cmd = [
            str(self.backend),
            "-i", str(middle),
            "-o", str(out),
        ]

        if asm is not None:
            cmd += ["-S", str(asm)]

        run(cmd, verbose=verbose)

    def compile_to_splobj(
        self,
        src: Path,
        obj: Path,
        work_dir: Path,
        *,
        verbose: bool,
    ) -> None:
        front = work_dir / f"{src.stem}.front"
        middle = work_dir / f"{src.stem}.middle"

        self.compile_to_middle(src, front, middle, verbose=verbose)

        run([
            str(self.backend),
            BACKEND_EMIT_OBJ_FLAG,
            "-i", str(middle),
            "-o", str(obj),
        ], verbose=verbose)

    def link_splobjs(self, objects: list[Path], out_obj: Path, *, verbose: bool) -> None:
        if not objects:
            fail("nothing to link")

        out_obj.parent.mkdir(parents=True, exist_ok=True)

        run([
            str(self.spl_link),
            "-o", str(out_obj),
            *map(str, objects),
        ], verbose=verbose)

    def convert_splobj_to_executable(self, obj: Path, out: Path, *, verbose: bool) -> None:
        out.parent.mkdir(parents=True, exist_ok=True)

        run([
            str(self.backend),
            BACKEND_CONVERT_OBJ_FLAG,
            "-i", str(obj),
            "-o", str(out),
        ], verbose=verbose)


def is_splobj(path: Path) -> bool:
    return path.suffix == ".splobj"


def compile_input_to_obj(
    toolchain: Toolchain,
    src: Path,
    obj: Path,
    work_dir: Path,
    *,
    verbose: bool,
) -> Path:
    if is_splobj(src):
        return src

    if src.suffix != ".lang":
        fail(f"unsupported input file type: {src}")

    toolchain.compile_to_splobj(src, obj, work_dir, verbose=verbose)
    return obj


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        prog=argv[0],
        description="SPL compiler driver: frontend -> midend -> backend, with optional .splobj linking.",
    )

    parser.add_argument(
        "inputs",
        nargs="+",
        type=Path,
        help="input .lang or .splobj files",
    )

    parser.add_argument(
        "-o", "--output",
        type=Path,
        help="output file. Default: input.out, input.splobj, or a.out",
    )

    parser.add_argument(
        "-l", "--link",
        action="append",
        default=[],
        type=Path,
        help="extra .splobj to link. Can be passed multiple times",
    )

    parser.add_argument(
        "-c", "--emit-obj",
        action="store_true",
        help="compile input .lang to .splobj and stop",
    )

    parser.add_argument(
        "-S", "--asm",
        type=Path,
        help="emit asm dump for simple non-link executable compilation",
    )

    parser.add_argument(
        "--bin-dir",
        type=Path,
        default=Path("build/bin"),
        help="directory with frontend, midend, backend, spl_link",
    )

    parser.add_argument(
        "--work-dir",
        type=Path,
        help="directory for temporary .front/.middle/.splobj files",
    )

    parser.add_argument(
        "--keep-temps",
        action="store_true",
        help="keep temporary work directory",
    )

    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="print commands before executing them",
    )

    return parser.parse_args(argv[1:])


def main(argv: list[str]) -> int:
    args = parse_args(argv)

    inputs: list[Path] = args.inputs
    links: list[Path] = args.link
    output: Path = args.output or default_output_for(inputs, args.emit_obj)

    for path in [*inputs, *links]:
        if not path.exists():
            fail(f"input does not exist: {path}")

    toolchain = Toolchain(args.bin_dir)

    link_mode = bool(links) or len(inputs) > 1

    if args.emit_obj and link_mode:
        fail("--emit-obj cannot be used with multiple inputs or -l")

    if args.emit_obj:
        if len(inputs) != 1:
            fail("--emit-obj expects exactly one input")

        src = inputs[0]
        if is_splobj(src):
            shutil.copyfile(src, output)
            return 0

        work_dir = args.work_dir or output.parent
        work_dir.mkdir(parents=True, exist_ok=True)

        toolchain.compile_to_splobj(src, output, work_dir, verbose=args.verbose)
        return 0

    if not link_mode:
        if len(inputs) != 1:
            fail("internal error: non-link mode with multiple inputs")

        src = inputs[0]

        if is_splobj(src):
            toolchain.convert_splobj_to_executable(src, output, verbose=args.verbose)
        else:
            asm = args.asm or replace_suffix(output, ".s")
            toolchain.compile_to_executable(src, output, asm, verbose=args.verbose)

        return 0

    if args.asm is not None:
        fail("-S/--asm is supported only for simple single-file compilation")

    if args.work_dir:
        args.work_dir.mkdir(parents=True, exist_ok=True)
        tmp_ctx = None
        work_root = args.work_dir
    else:
        tmp_ctx = tempfile.TemporaryDirectory(prefix="splc_")
        work_root = Path(tmp_ctx.name)

    try:
        objects: list[Path] = []

        for index, src in enumerate(inputs):
            obj = work_root / f"input_{index}_{src.stem}.splobj"
            objects.append(compile_input_to_obj(
                toolchain,
                src,
                obj,
                work_root,
                verbose=args.verbose,
            ))

        objects.extend(links)

        merged_obj = work_root / "linked.splobj"
        toolchain.link_splobjs(objects, merged_obj, verbose=args.verbose)
        toolchain.convert_splobj_to_executable(merged_obj, output, verbose=args.verbose)

        if args.keep_temps and tmp_ctx is not None:
            print(f"splc: kept temps in {work_root}", file=sys.stderr)
            tmp_ctx = None

        return 0
    finally:
        if tmp_ctx is not None:
            tmp_ctx.cleanup()


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
