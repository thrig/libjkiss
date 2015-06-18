#ifndef _H_LIBJKISS_H_
#define _H_LIBJKISS_H_

#ifdef JKISS_LIBBSD
#include <bsd/stdlib.h>
#endif

#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

void jkiss64_init(void);
uint64_t jkiss64_rand(void);

#endif
