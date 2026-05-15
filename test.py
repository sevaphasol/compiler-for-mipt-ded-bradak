#!/usr/bin/env python3
"""
Auto‑build + test runner for .lang files.
"""

import subprocess
import sys
import signal
from pathlib import Path

# --------------------------------------------------------------------
PROJECT_ROOT = Path(__file__).resolve().parent
EXAMPLES_DIR = PROJECT_ROOT / "examples"
OUTPUT_DIR   = PROJECT_ROOT / "test_output"
RUN_SH       = PROJECT_ROOT / "run.sh"

COMPILE_TIMEOUT = 30       # seconds
LAUNCH_TIMEOUT  = 1        # seconds
# --------------------------------------------------------------------

def compile_file(src: Path, dst: Path) -> subprocess.CompletedProcess:
    """Run run.sh <src> <dst> and return the process result."""
    cmd = [str(RUN_SH), str(src), str(dst)]
    try:
        return subprocess.run(cmd, capture_output=True, text=True,
                              timeout=COMPILE_TIMEOUT)
    except subprocess.TimeoutExpired:
        return subprocess.CompletedProcess(cmd, -1, "", "Compilation timed out")


def launch_executable(exe: Path) -> tuple[int, str]:
    """
    Try to run the executable.
    Returns (exit_code, message).
    """
    try:
        proc = subprocess.run([str(exe)], capture_output=True, text=True,
                              timeout=LAUNCH_TIMEOUT, stdin=subprocess.DEVNULL)
        if proc.returncode >= 0:
            return proc.returncode, "OK"
        else:
            sig = -proc.returncode
            try:
                sig_name = signal.Signals(sig).name
            except ValueError:
                sig_name = f"signal {sig}"
            return proc.returncode, f"killed by {sig_name}"
    except subprocess.TimeoutExpired:
        return -999, "timed out (killed)"
    except FileNotFoundError:
        return -1, "executable not found"
    except Exception as e:
        return -2, f"unexpected error: {e}"


def main():
    # 1. Build
    print("=== Building compiler ===")
    result = subprocess.run(["make"], cwd=PROJECT_ROOT, capture_output=True,
                            text=True)
    if result.returncode != 0:
        print("Build FAILED.")
        print("stdout:", result.stdout)
        print("stderr:", result.stderr)
        sys.exit(1)
    print("Build OK.\n")

    # 2. Prepare output directory
    OUTPUT_DIR.mkdir(exist_ok=True)

    # 3. Collect .lang files
    lang_files = sorted(EXAMPLES_DIR.rglob("*.lang"))
    if not lang_files:
        print(f"No .lang files found in {EXAMPLES_DIR}")
        return

    total = len(lang_files)
    compile_ok = 0
    compile_fail = 0
    launch_ok = 0
    launch_fail = 0
    
    # List to track failed test paths
    failed_test_paths = []

    print(f"=== Processing {total} file(s) ===")

    for lang_file in lang_files:
        rel = lang_file.relative_to(EXAMPLES_DIR)
        out_file = OUTPUT_DIR / rel.with_suffix(".out")
        out_file.parent.mkdir(parents=True, exist_ok=True)

        print(f"\n[{lang_file}]")

        # Compile
        compile_proc = compile_file(lang_file, out_file)
        if compile_proc.returncode == 0 and out_file.exists():
            compile_ok += 1
            print("  Compilation: OK")

            # Launch
            exit_code, msg = launch_executable(out_file)
            # Treat exit_code 0 as success, others as failure
            if exit_code == 0:
                launch_ok += 1
                print(f"  Launch: OK (exit code {exit_code})")
            else:
                launch_fail += 1
                failed_test_paths.append(str(lang_file))
                print(f"  Launch: FAILED (exit code {exit_code}, msg: {msg})")
        else:
            compile_fail += 1
            failed_test_paths.append(str(lang_file))
            print("  Compilation: FAILED")
            if compile_proc.stdout:
                for line in compile_proc.stdout.splitlines():
                    print(f"    stdout: {line}")
            if compile_proc.stderr:
                for line in compile_proc.stderr.splitlines():
                    print(f"    stderr: {line}")

    # Summary
    print("\n" + "=" * 60)
    print("SUMMARY")
    print(f"Total files:        {total}")
    print(f"Compilation passed: {compile_ok}")
    print(f"Compilation failed: {compile_fail}")
    print(f"Launch passed:      {launch_ok}")
    print(f"Launch failed:      {launch_fail}")

    # Print failed paths at the very end
    if failed_test_paths:
        print("\nFAILED TEST PATHS:")
        for path in failed_test_paths:
            print(f"  {path}")
        sys.exit(1)
    else:
        print("\nALL TESTS PASSED.")

if __name__ == "__main__":
    main()