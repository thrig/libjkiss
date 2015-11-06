/*
 # The primary functions of this library are jkiss_init() and
 # jkiss_rand(); the first of these should be called once, perhaps in
 # the main thread before the others are started, and the second is used
 # to obtain a random value. Each thread will use its own seed, set
 # randomly via arc4random(3) or via the /dev/random device. The period
 # of this RNG function is ~ 2**250, so repeats due to long running
 # processes that make heavy use of the RNG are unlikely. Consult
 # jkiss(3) for additional documentation, or search under the eg/
 # directory for example code.
 */

#include "jkiss.h"

#ifndef DEV_RANDOM
#define DEV_RANDOM "/dev/random"
#endif

void (*Re_Seed) (jkiss64_seed_t * seed);
pthread_key_t Thread_Seed;

void jkiss64_seeder(jkiss64_seed_t * seed);
void jkiss64_thread_cleanup(void *seed);

/************************************************************************
 *
 * Public Functions
 *
 */

int jkiss64_init(void (*seed_func) (jkiss64_seed_t * seed))
{
    int r;
    if ((r = pthread_key_create(&Thread_Seed, jkiss64_thread_cleanup)) != 0)
        return r;

    Re_Seed = seed_func ? seed_func : jkiss64_seeder;
    return 0;
}

uint64_t jkiss64_rand(void)
{
    uint64_t t;
    jkiss64_seed_t *seed = pthread_getspecific(Thread_Seed);

    if (!seed) {
        if ((seed = malloc(sizeof(jkiss64_seed_t))) == NULL)
            abort();
        pthread_setspecific(Thread_Seed, seed);
        seed->seeded = false;
    }
    if (!seed->seeded) {
        Re_Seed(seed);
        seed->seeded = true;
    }

    seed->x = 1490024343005336237ULL * seed->x + 123456789;
    seed->y ^= seed->y << 21;
    seed->y ^= seed->y >> 17;
    seed->y ^= seed->y << 30;
    t = 4294584393ULL * seed->z1 + seed->c1;
    seed->c1 = t >> 32;
    seed->z1 = (uint32_t) t;
    t = 4246477509ULL * seed->z2 + seed->c2;
    seed->c2 = t >> 32;
    seed->z2 = (uint32_t) t;
    return seed->x + seed->y + seed->z1 + ((uint64_t) seed->z2 << 32);
}

void jkiss64_reseed(void)
{
    jkiss64_seed_t *seed = pthread_getspecific(Thread_Seed);
    if (seed)
        seed->seeded = false;
}

uint64_t jkiss64_uniform(uint64_t upper_bound)
{
    uint64_t max, rand;

    if (upper_bound == 0) abort();

    max = UINT64_MAX / upper_bound * upper_bound;

    rand = jkiss64_rand();
    while (rand > max) {
        rand = jkiss64_rand();
    }
    return rand % upper_bound;
}

/************************************************************************
 *
 * Private Functions
 *
 */

void jkiss64_seeder(jkiss64_seed_t * seed)
{
#ifdef HAVE_ARC4RANDOM
    seed->x = arc4random() | ((uint64_t) arc4random() << 32);
    seed->y = 0;
    while (seed->y == 0) {
        seed->y = arc4random() | ((uint64_t) arc4random() << 32);
    }
    seed->c1 = seed->z1 = 0;
    while (seed->c1 == 0 && seed->z1 == 0) {
        seed->c1 = arc4random();
        seed->z1 = arc4random();
    }
    seed->c2 = seed->z2 = 0;
    while (seed->c2 == 0 && seed->z2 == 0) {
        seed->c2 = arc4random();
        seed->z2 = arc4random();
    }
#else
    int fd = open(DEV_RANDOM, O_RDONLY);
    if (fd == -1)
        abort();
    if (read(fd, &seed->x, sizeof(uint64_t)) != sizeof(uint64_t))
        abort();
    while (seed->y == 0) {
        if (read(fd, &seed->y, sizeof(uint64_t)) != sizeof(uint64_t))
            abort();
    }
    seed->c1 = seed->z1 = 0;
    while (seed->c1 == 0 && seed->z1 == 0) {
        if (read(fd, &seed->c1, sizeof(uint32_t)) != sizeof(uint32_t))
            abort();
        if (read(fd, &seed->z1, sizeof(uint32_t)) != sizeof(uint32_t))
            abort();
    }
    seed->c2 = seed->z2 = 0;
    while (seed->c2 == 0 && seed->z2 == 0) {
        if (read(fd, &seed->c2, sizeof(uint32_t)) != sizeof(uint32_t))
            abort();
        if (read(fd, &seed->z2, sizeof(uint32_t)) != sizeof(uint32_t))
            abort();
    }
#endif
}

void jkiss64_thread_cleanup(void *seed)
{
    free(seed);
}
