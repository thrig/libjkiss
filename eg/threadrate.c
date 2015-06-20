/* Test rate at which pRNG numbers can be generated. 

On a "8" processor OpenBSD 5.7 system (Intel(R) Core(TM) i7-2600 CPU @
3.40GHz), using pthread_mutex_* locks around a single globally shared
seed, the rate for pRNG calls in a tight loop was:

1 25425262.948
2 74013.209
4 37971.944
8 18674.673
16 9384.903

There is a huge drop between one and two threads, then downhill at a slower
rate from there - but this is worst case, where the threads are doing
nothing but nomming on random numbers. (Standard deviation also rises with
increased numbers of threads, but that's not unexpected.)

So instead went with a seed-per-thread implementation, which does not slow
down quite so significantly as the above does.

  for t in 1 2 4 8 16; do
    ./threadrate -c 1000000 -t $t \
    | awk '{print $NF}' \
    | r-fu summary - \
    | grep elem \
    | awk '{print $2, $4}';
  done

*/

#include <sys/time.h>

#include <err.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

// https://github.com/thrig/libjkiss (this library!)
#include <jkiss.h>

#define NSEC_IN_SEC 1000000000

unsigned long Flag_Count;       // -c
unsigned long Flag_Threads;     // -t

unsigned long Threads_Completed;

pthread_mutex_t Lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t Job_Done = PTHREAD_COND_INITIALIZER;

long double *Thread_Rates;

void emit_help(void);
void *worker(void *ratearg);

int main(int argc, char *argv[])
{
    int ch;
    pthread_t *tids;
    struct timespec wait;

    while ((ch = getopt(argc, argv, "c:h?t:")) != -1) {
        switch (ch) {

        case 'c':
            Flag_Count = flagtoul(ch, optarg, 1UL, ULONG_MAX);
            break;

        case 't':
            Flag_Threads = flagtoul(ch, optarg, 1UL, ULONG_MAX);
            break;

        case 'h':
        case '?':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if (Flag_Count == 0 || Flag_Threads == 0)
        emit_help();

    jkiss64_init(NULL);

    if ((tids = calloc(sizeof(pthread_t), Flag_Threads)) == NULL)
        err(EX_OSERR, "could not calloc() threads list");

    if ((Thread_Rates = calloc(sizeof(long double), Flag_Threads)) == NULL)
        err(EX_OSERR, "could not calloc() thread rates list");

    for (unsigned long i = 0; i < Flag_Threads; i++) {
        if (pthread_create(&tids[i], NULL, worker, &Thread_Rates[i]) != 0)
            err(EX_OSERR, "could not pthread_create() thread %lu", i);
    }

    wait.tv_sec = 3;
    wait.tv_nsec = 2500000;

    for (;;) {
        pthread_mutex_lock(&Lock);
        if (Threads_Completed == Flag_Threads)
            break;
        pthread_cond_timedwait(&Job_Done, &Lock, &wait);
        pthread_mutex_unlock(&Lock);
    }

    for (unsigned long i = 0; i < Flag_Threads; i++) {
        printf("thread[%lu] %.6Lf\n", i, Thread_Rates[i]);
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: threadrate -c count -t threads\n");
    exit(EX_USAGE);
}

void *worker(void *ratearg)
{
    struct timespec before, after;
    long double delta_t, *ratep;
    uint64_t r;

    ratep = ratearg;

    if (clock_gettime(CLOCK_REALTIME, &before) == -1)
        err(EX_OSERR, "clock_gettime() failed");

    // this is a worst case; real-world apps would presumably do other
    // things instead of only leaning on the get-a-rnd-value call
    for (unsigned long c = 0; c < Flag_Count; c++) {
        r = jkiss64_rand();
    }

    if (clock_gettime(CLOCK_REALTIME, &after) == -1)
        err(EX_OSERR, "clock_gettime() failed");
    /* hopefully close enough via double conversion; the alternative would
     * be to trade off timer resolution against the total time thereby
     * possible to measure, which I have not looked into. */
    delta_t =
        (after.tv_sec - before.tv_sec) + (after.tv_nsec -
                                          before.tv_nsec) /
        (long double) NSEC_IN_SEC;
    *ratep = Flag_Count / delta_t;

    pthread_mutex_lock(&Lock);
    Threads_Completed++;
    pthread_mutex_unlock(&Lock);
    pthread_cond_signal(&Job_Done);

    return (void *) 0;
}
