#include <stdint.h>
#include <kernel/tty.h>
#include <kernel/timer.h>
#include <kernel/keyboard.h>
#include <stdio.h>
#include "pic.h"

struct register_frame {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no;
    uint32_t err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

void isr_handler(struct register_frame* r) {
    printf("CPU exception %u\n", r->int_no);
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

void irq_handler(struct register_frame* r) {

    if (r->int_no == 32) {
        timer_handle_irq();
    }

    if (r->int_no == 33) {
        keyboard_handle_irq();
    }

    pic_send_eoi((uint8_t)(r->int_no - 32));
}
