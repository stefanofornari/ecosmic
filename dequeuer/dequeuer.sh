#!/bin/bash

COMMAND_TO_RUN="$1"
QUEUE_DIR="$2"

if [ -z "$COMMAND_TO_RUN" ] || [ -z "$QUEUE_DIR" ]; then
  echo "Usage: $0 <command_to_run> <queue_directory>"
  exit 1
fi

IN_DIR="${QUEUE_DIR}/in"
OUT_DIR="${QUEUE_DIR}/out"
DONE_DIR="${QUEUE_DIR}/done"
FAILED_DIR="${QUEUE_DIR}/failed"

mkdir -p "$IN_DIR" "$OUT_DIR" "$DONE_DIR" "$FAILED_DIR"

while true; do
  # Find the first .cpe file in any subdirectory of IN_DIR
  FILE=$(find "$IN_DIR" -type f -name "*.cpe" | head -n 1)

  if [ -n "$FILE" ]; then
    echo "Processing file: $FILE"

    # Extract norad_id from the file path (assuming it's the immediate parent directory of the file)
    NORAD_ID=$(basename $(dirname "$FILE"))
    FILENAME=$(basename "$FILE" .cpe)

    # Create output, done, and failed directories for the norad_id if they don't exist
    mkdir -p "${OUT_DIR}/${NORAD_ID}"
    mkdir -p "${DONE_DIR}/${NORAD_ID}"
    mkdir -p "${FAILED_DIR}/${NORAD_ID}"

    # Cat content, pipe to command, and capture output
    COMMAND_OUTPUT=$(cat "$FILE" | $COMMAND_TO_RUN)
    EXIT_CODE=$?

    if [ $EXIT_CODE -eq 0 ]; then
      # Write output to .out file
      echo "$COMMAND_OUTPUT" > "${OUT_DIR}/${NORAD_ID}/${FILENAME}.out"
      echo "Output written to: ${OUT_DIR}/${NORAD_ID}/${FILENAME}.out"

      # Move processed file to done directory
      mv "$FILE" "${DONE_DIR}/${NORAD_ID}/"
      echo "File moved to: ${DONE_DIR}/${NORAD_ID}/"
    else
      echo "Command failed with exit code $EXIT_CODE for file: $FILE"
      # Move failed file to failed directory
      mv "$FILE" "${FAILED_DIR}/${NORAD_ID}/"
      echo "File moved to: ${FAILED_DIR}/${NORAD_ID}/"
    fi
  else
    echo "No .cpe files found. Waiting..."
  fi

  sleep 1
done
