#!/bin/sh
set -e
. ./config.sh

log_message "$LOG_STEP" "Validating bootable image dependencies..."
. ./iso.sh || { log_message "$LOG_ERR" "ISO validation failed. Cannot boot QEMU."; exit 1; }

ARCH=$(./target-triplet-to-arch.sh "$HOST")

log_message "$LOG_STEP" "Launching QEMU Emulator ($ARCH)..."
qemu-system-"$ARCH" -cdrom "$ISO_IMAGE" -display cocoa,zoom-to-fit=on
