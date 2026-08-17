// kernel/arch/i386/io.h
#ifndef ARCH_I386_IO_H
#define ARCH_I386_IO_H

#include <stdint.h>

// helper function to read a byte from a hardware port (assembly wrapper)
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// helper function to write a byte to a hardware port (assembly wrapper)
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

#endif