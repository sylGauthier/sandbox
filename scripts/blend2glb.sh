#!/bin/bash

if [ -z "$1" -o -z "$2" ] ; then
    printf "Usage: $0 <file.blend> <output.ogex>\n"
    exit 1
fi

EXPORT_SCRIPT="$(dirname "$0")/export.py"
TMP_EXPORT_DIR="$(mktemp -d)"
NAME="$(basename "$1")"
NAME="${NAME%.blend}"

blender "$1" --background --python "$EXPORT_SCRIPT" -- "$2" || exit 1
