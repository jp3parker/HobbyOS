#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include <stdint.h>

// This function will sit in an infinite loop, reading keys and printing them
void keyboard_poll_loop(void);

#endif
