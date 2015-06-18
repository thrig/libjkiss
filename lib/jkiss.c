/*
 # Public domain JLKISS64 implementation - KISS pseudo-random number
 # generator (pRNG) with 64-bit operations and thread-independent seeding.
 # Each pthread will use a different seed set. The period of this pRNG is
 # ~2**250 so only extremely long-running threads that make extremely heavy
 # use of the pRNG will need to reseed? (NOTE - there is no re-seed
 # support, though that would be easy to add via a call that NULLs the
 # Thread_Seed specific to the thread.)
 */

#include "jkiss.h"

void jkiss64_thread_cleanup(void *key);

pthread_key_t Thread_Seed;
struct jkiss64_seed {
    uint64_t x;
    uint64_t y;
    uint32_t c1;
    uint32_t c2;
    uint32_t z1;
    uint32_t z2;
};

typedef jkiss64_seed_t struct jkiss64_seed

void jkiss64_init(void)
{
    // TODO tho would presumably need destructor to free seed struct?
    pthread_key_create(&Thread_Seed, jkiss64_thread_cleanup);
}

uint64_t jkiss64_rand(void)
{
    uint64_t t;
    jkiss64_seed_t *seed = pthread_getspecific(Thread_Seed);

    if (!seed) {
        if ((seed = malloc(sizeof(jkiss64_seed_t))) == NULL)
            abort();
        pthread_setspecific(Thread_Seed, seed);
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
        // TODO must also detect for this for portability reasons
        int fd = open("/dev/random", O_RDONLY);
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

void jkiss64_thread_cleanup(void *key)
{
    free(key);
}
