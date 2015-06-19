#ifndef _H_LIBJKISS_H_
#define _H_LIBJKISS_H_

#ifdef JKISS_LIBBSD
#include <bsd/stdlib.h>
#endif

#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

int jkiss64_init(void *(*seed_func) (jkiss64_seed_t * seed));
uint64_t jkiss64_rand(void);
void jkiss64_reseed(void);

#endif
