#!/bin/sh
set -e
. ./config.sh

log_message "$LOG_STEP" "Starting cleanup process..."

for PROJECT in $PROJECTS; do
  log_message "$LOG_INFO" "Cleaning directory: $PROJECT"

  cd "$PROJECT"
  $MAKE clean || { log_message "$LOG_ERR" "Failed to clean $PROJECT"; exit 1; }
  cd ..
done

log_message "$LOG_INFO" "Removing root-level build directories..."
rm -rf sysroot
rm -rf isodir
rm -rf myos.iso

log_message "$LOG_STEP" "Cleanup successful."
