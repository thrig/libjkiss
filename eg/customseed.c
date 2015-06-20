/* libjkiss example; threads each with a custom seed set by set_seed, below. */

#include <sys/time.h>

#include <err.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

// https://github.com/thrig/libjkiss (this library!)
#include <jkiss.h>

#define NUM_THREADS 2U
#define ITERATIONS 10U

pthread_mutex_t Lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t Job_Done = PTHREAD_COND_INITIALIZER;

int Threads_Completed;

void set_seed(jkiss64_seed_t * seed);
void *worker(void *threadnum);

int main(void)
{
    pthread_t *tids;
    struct timespec wait;

    setlinebuf(stdout);

    jkiss64_init(set_seed);

    if ((tids = calloc(sizeof(pthread_t), (size_t) NUM_THREADS)) == NULL)
        err(EX_OSERR, "could not calloc() threads list");

    for (unsigned int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&tids[i], NULL, worker, (void *) i) != 0)
            err(EX_OSERR, "could not pthread_create() thread %d", i);
    }

    wait.tv_sec = 3;
    wait.tv_nsec = 45;

    for (;;) {
        pthread_mutex_lock(&Lock);
        if (Threads_Completed == NUM_THREADS)
            break;
        pthread_cond_timedwait(&Job_Done, &Lock, &wait);
        pthread_mutex_unlock(&Lock);
    }

    exit(EXIT_SUCCESS);
}

void set_seed(jkiss64_seed_t * seed)
{
    seed->x = 1;
    seed->y = 1;
    seed->c1 = 6543217;
    seed->z1 = 43219876;
    seed->c2 = 1732654;
    seed->z2 = 21987643;
}

void *worker(void *threadnum)
{
    int x = (int) threadnum;

    for (unsigned int i = 0; i < ITERATIONS; i++) {
        printf("thread %d rand %llu\n", x, jkiss64_rand());
    }

    pthread_mutex_lock(&Lock);
    Threads_Completed++;
    pthread_mutex_unlock(&Lock);
    pthread_cond_signal(&Job_Done);

    return (void *) 0;
}
