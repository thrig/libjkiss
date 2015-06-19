/*
 # Public domain JLKISS64 implementation - KISS pseudo-random number
 # generator (pRNG) with 64-bit operations and thread-independent seeding.
 # Each pthread will use a different seed set. The period of this pRNG is
 # ~2**250 so only extremely long-running threads that make extremely heavy
 # use of the pRNG will likely need to reseed.
 */

#include "jkiss.h"

#ifndef DEV_RANDOM
#define DEV_RANDOM "/dev/random"
#endif

void jkiss64_seeder(jkiss64_seed_t * seed);
void jkiss64_thread_cleanup(void *seed);

void *Re_Seed;
pthread_key_t Thread_Seed;
typedef struct jkiss64_seed {
    bool seeded;
    uint64_t x;
    uint64_t y;
    uint32_t c1;
    uint32_t c2;
    uint32_t z1;
    uint32_t z2;
} jkiss64_seed_t;

/************************************************************************
 *
 * Public Functions
 *
 */

int jkiss64_init(void *(*seed_func) (jkiss64_seed_t * seed))
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

/************************************************************************
 *
 * Private Functions
 *
 */

// may need second void * arg for arbitrary user-supplied stuff?
void jkiss64_seeder(jkiss64_seed_t * seed)
{
    seed->c1 = 6543217;
    seed->c2 = 43219876;
    seed->z1 = 1732654;
    seed->z2 = 21987643;
    seed->y = 0;

#ifdef HAVE_ARC4RANDOM
    seed->x = arc4random() | ((uint64_t) arc4random() << 32);
    while (seed->y == 0) {
        seed->y = arc4random() | ((uint64_t) arc4random() << 32);
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
#endif
}

void jkiss64_thread_cleanup(void *seed)
{
    free(seed);
}
