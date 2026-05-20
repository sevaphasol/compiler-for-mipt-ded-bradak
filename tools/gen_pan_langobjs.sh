#!/usr/bin/env bash
set -euo pipefail

PAN_DIR=""
OUT_DIR=""
BIN_DIR=""

usage() {
    cat >&2 <<EOF
Usage:
  $0 --pan-dir <dir-with-pan-files> --out-dir <generated-dir> --bin-dir <build-bin-dir>

Example:
  $0 --pan-dir ./pan --out-dir ./build/generated/pan --bin-dir ./build/bin
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --pan-dir)
            PAN_DIR="${2:-}"
            shift 2
            ;;
        --out-dir)
            OUT_DIR="${2:-}"
            shift 2
            ;;
        --bin-dir)
            BIN_DIR="${2:-}"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            usage
            exit 1
            ;;
    esac
done

if [[ -z "$PAN_DIR" || -z "$OUT_DIR" || -z "$BIN_DIR" ]]; then
    usage
    exit 1
fi

PAN_DIR="$(realpath "$PAN_DIR")"
OUT_DIR="$(realpath -m "$OUT_DIR")"
BIN_DIR="$(realpath "$BIN_DIR")"

PAN2LANG="$BIN_DIR/pan2lang"
FRONTEND="$BIN_DIR/frontend"
MIDEND="$BIN_DIR/midend"
BACKEND="$BIN_DIR/backend"

for tool in "$PAN2LANG" "$FRONTEND" "$MIDEND" "$BACKEND"; do
    if [[ ! -x "$tool" ]]; then
        echo "error: tool not found or not executable: $tool" >&2
        exit 1
    fi
done

mkdir -p "$OUT_DIR"

count=0

while IFS= read -r -d '' pan_file; do
    rel_path="${pan_file#"$PAN_DIR"/}"
    rel_dir="$(dirname "$rel_path")"
    name="$(basename "$rel_path" .pan)"

    if [[ "$rel_dir" == "." ]]; then
        out_dir="$OUT_DIR"
    else
        out_dir="$OUT_DIR/$rel_dir"
    fi

    mkdir -p "$out_dir"

    lang="$out_dir/$name.lang"
    langinc="$out_dir/$name.langinc"
    front="$out_dir/$name.front"
    middle="$out_dir/$name.middle"
    splobj="$out_dir/$name.splobj"

    echo "PAN: $rel_path"
    echo "  -> $lang"
    echo "  -> $langinc"
    echo "  -> $splobj"

    "$PAN2LANG" \
        -i "$pan_file" \
        --lang "$lang" \
        --langinc "$langinc"

    "$FRONTEND" \
        "$lang" \
        "$front"

    "$MIDEND" \
        -i "$front" \
        -o "$middle"

    "$BACKEND" \
        --emit-obj \
        -i "$middle" \
        -o "$splobj"

    count=$((count + 1))
done < <(find "$PAN_DIR" -type f -name '*.pan' -print0)

echo "generated $count .splobj file(s)"
