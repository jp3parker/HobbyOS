#!/bin/sh
set -e
. ./config.sh

log_message "$LOG_STEP" "Starting cleanup process..."

for PROJECT in $PROJECTS; do
  log_message "$LOG_INFO" "Cleaning directory: $PROJECT"
  project_make "$PROJECT" $MAKE clean || { log_message "$LOG_ERR" "Failed to clean $PROJECT"; exit 1; }
done

log_message "$LOG_INFO" "Removing root-level build directories..."
rm -rf "$SYSROOT"
rm -rf "$ISO_DIR"
rm -f "$ISO_IMAGE"

log_message "$LOG_STEP" "Cleanup successful."
