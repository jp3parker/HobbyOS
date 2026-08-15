#!/bin/sh
set -e
. ./config.sh

log_message "$LOG_STEP" "Preparing system headers..."
mkdir -p "$SYSROOT"

for PROJECT in $SYSTEM_HEADER_PROJECTS; do
  log_message "$LOG_INFO" "Installing headers for: $PROJECT"

  cd "$PROJECT"
  DESTDIR="$SYSROOT" $MAKE install-headers || { 
    log_message "$LOG_ERR" "Header installation failed in $PROJECT"
    exit 1
  }
  cd ..
done

log_message "$LOG_STEP" "System headers installed to sysroot."
