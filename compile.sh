#!/bin/bash

if [ "$#" -eq 2 ]; then
    SRCFILE="$1"
    OUTFILE="$2"
elif [ "$#" -eq 1 ]; then
    SRCFILE="$1"
    OUTFILE="${SRCFILE%.*}.out"
else
    echo "Usage: $0 <input-file> [output-file]"
    exit 1
fi

ASMFILE="${OUTFILE%.*}.s"
FRONTFILE="${OUTFILE%.*}.front"
MIDDLEFILE="${OUTFILE%.*}.middle"

# TMPFILE_FRONT=$(mktemp frontend_tmp.XXXXXX)
# TMPFILE_MIDDLE=$(mktemp middleend_tmp.XXXXXX)

build/bin/frontend "$SRCFILE" "$FRONTFILE"
build/bin/midend -i "$FRONTFILE" -o "$MIDDLEFILE"
build/bin/backend -i "$MIDDLEFILE" -o "$OUTFILE" -S "$ASMFILE"

# rm -f "$TMPFILE_FRONT"
# rm -f "$TMPFILE_MIDDLE"
