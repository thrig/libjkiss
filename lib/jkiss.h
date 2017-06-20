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

typedef struct jkiss64_seed {
    uint64_t x;
    uint64_t y;
    uint32_t c1;
    uint32_t c2;
    uint32_t z1;
    uint32_t z2;
} jkiss64_seed_t;

jkiss64_seed_t * jkiss64_getseed(void);
int jkiss64_init(void (*seed_func) (jkiss64_seed_t * seed));
int jkiss64_init_thread(void);
uint64_t jkiss64_rand(void);
void jkiss64_reseed(void);
uint64_t jkiss64_uniform(uint64_t upper_bound);

#endif
