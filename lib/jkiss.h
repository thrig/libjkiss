#ifndef _H_LIBJKISS_H_
#define _H_LIBJKISS_H_

#ifdef __linux__ && HAVE_ARC4RANDOM
#include <bsd/stdlib.h>
#endif

// TODO only needed if using /dev/random
#include <fcntl.h>

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

void jkiss64_init(void);
uint64_t jkiss64_rand(void);
// TODO uniform (though modulo bias most likely irrelevant unless selecting
// up near UINT64_MAX) perhaps other utility functions

#endif
