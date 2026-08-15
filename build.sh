#!/bin/sh
set -e
. ./config.sh

. ./headers.sh || { log_message "$LOG_ERR" "Build process failed headers dependency. Aborting."; exit 1; }

for PROJECT in $PROJECTS; do
  log_message "$LOG_INFO" "Building: $PROJECT"

  cd $PROJECT
  DESTDIR="$SYSROOT" $MAKE install || { 
    log_message "$LOG_ERR" "'make install' failed in $PROJECT"
    exit 1
  }
  cd ..
done

log_message "$LOG_STEP" "Build process completed."
