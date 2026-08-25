#ifndef _KERNEL_TIMER_H
#define _KERNEL_TIMER_H

#include <stdint.h>

void timer_install(void);
void timer_handle_irq(void);
uint32_t timer_ticks(void);
void timer_wait(uint32_t ticks);

#endif
