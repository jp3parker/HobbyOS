#!/bin/sh
set -e
. ./config.sh

log_message "$LOG_STEP" "Triggering OS build compilation..."
. ./build.sh || { log_message "$LOG_ERR" "Compilation failed. Aborting ISO creation."; exit 1; }

log_message "$LOG_STEP" "Generating bootable live ISO..."
mkdir -p "$ISO_DIR/boot/grub"

cp "$SYSROOT/boot/myos.kernel" "$ISO_DIR/boot/myos.kernel" || { log_message "$LOG_ERR" "Could not find myos.kernel in sysroot."; exit 1; }
cat > "$ISO_DIR/boot/grub/grub.cfg" << EOF
menuentry "myos" {
	multiboot /boot/myos.kernel
}
EOF

grub-mkrescue -o "$ISO_IMAGE" "$ISO_DIR" || { log_message "$LOG_ERR" "grub-mkrescue failed to assemble ISO image."; exit 1; }

log_message "$LOG_STEP" "Successfully generated: $ISO_IMAGE"
