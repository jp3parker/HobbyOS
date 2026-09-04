#ifndef _KERNEL_PLATFORM_H
#define _KERNEL_PLATFORM_H

#include <stddef.h>
#include <stdint.h>

void kernel_arch_initialize(uint32_t multiboot_magic, uintptr_t multiboot_info);

#endif
