#!/bin/sh
set -e
. ./config.sh

log_message "$LOG_STEP" "Triggering OS build compilation..."
. ./build.sh || { log_message "$LOG_ERR" "Compilation failed. Aborting ISO creation."; exit 1; }

log_message "$LOG_STEP" "Generating bootable live ISO..."
mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp $SYSROOT/boot/myos.kernel isodir/boot/myos.kernel || { log_message "$LOG_ERR" "Could not find myos.kernel in sysroot."; exit 1; }
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "myos" {
	multiboot /boot/myos.kernel
}
EOF

grub-mkrescue -o myos.iso isodir || { log_message "$LOG_ERR" "grub-mkrescue failed to assemble ISO image."; exit 1; }

log_message "$LOG_STEP" "Successfully generated: myos.iso"
