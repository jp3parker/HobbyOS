#include <stdint.h>

#include <kernel/timer.h>

#include "io.h"

#define PIT_FREQUENCY 100U
#define PIT_INPUT_CLOCK 1193182U

static volatile uint32_t timer_tick_count = 0;

void timer_install(void) {
    uint32_t divisor = PIT_INPUT_CLOCK / PIT_FREQUENCY;

    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

void timer_handle_irq(void) {
    ++timer_tick_count;
}

uint32_t timer_ticks(void) {
    return timer_tick_count;
}

void timer_wait(uint32_t ticks) {
    uint32_t start = timer_ticks();

    while ((uint32_t)(timer_ticks() - start) < ticks) {
        __asm__ volatile ("hlt");
    }
}

void timer_wait_until(uint32_t deadline) {
    while ((int32_t)(deadline - timer_ticks()) > 0) {
        __asm__ volatile ("hlt");
    }
}
