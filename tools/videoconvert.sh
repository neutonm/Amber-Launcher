#!/bin/bash

# This script converts a video to Theora format (.ogv) using ffmpeg.
# Usage: ./convert_to_theora.sh input_video [output_video]

if ! command -v ffmpeg &> /dev/null; then
    echo "Error: ffmpeg is not installed. Please install ffmpeg."
    exit 1
fi

if [ $# -lt 1 ] || [ $# -gt 2 ]; then
    echo "Usage: $0 input_video [output_video]"
    exit 1
fi

INPUT="$1"

if [ ! -f "$INPUT" ]; then
    echo "Error: Input file '$INPUT' does not exist."
    exit 1
fi

# If an output file name is not provided, create one by replacing the extension with .ogv
if [ $# -eq 2 ]; then
    OUTPUT="$2"
else
    BASENAME=$(basename "$INPUT")
    FILENAME="${BASENAME%.*}"
    OUTPUT="${FILENAME}.ogv"
fi

ffmpeg -i "$INPUT" -c:v libtheora -qscale:v 7 -c:a libvorbis -qscale:a 5 "$OUTPUT"

if [ $? -eq 0 ]; then
    echo "Conversion complete: $OUTPUT"
else
    echo "An error occurred during conversion."
fi
