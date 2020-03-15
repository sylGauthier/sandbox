#!/bin/bash

if [ -z "$1" -o -z "$2" ] ; then
    printf "Usage: $0 <file.blend> <output.ogex>\n"
    exit 1
fi

EXPORT_SCRIPT="$(dirname "$0")/export.py"
TMP_EXPORT_DIR="$(mktemp -d)"
NAME="$(basename "$1")"
NAME="${NAME%.blend}"

blender "$1" --background --python "$EXPORT_SCRIPT" -- "$TMP_EXPORT_DIR" || exit 1

ogexfix anim merge $(echo $TMP_EXPORT_DIR/$NAME*.ogex | sed 's/\([^ ]*-\([^ ]*\).ogex\)/\1:\2/g') | ogexfix texture changepath "../textures" > "$2"

rm -r "$TMP_EXPORT_DIR"
