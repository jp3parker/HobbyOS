#ifndef _UNISTD_H
#define _UNISTD_H 1

#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

unsigned int sleep(unsigned int);
unsigned int sleep_ms(unsigned int);

#ifdef __cplusplus
}
#endif

#endif
