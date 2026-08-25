#include <stdint.h>
#include <unistd.h>

extern uint32_t timer_ticks(void);
extern void timer_wait(uint32_t ticks);

unsigned int sleep(unsigned int seconds) {
    timer_wait(seconds * 100U);
    return 0;
}

unsigned int sleep_ms(unsigned int milliseconds) {
    const uint32_t tick_ms = 10U;
    uint32_t ticks = milliseconds / tick_ms;

    if ((milliseconds % tick_ms) != 0U) {
        ++ticks;
    }

    timer_wait(ticks);
    return 0;
}
