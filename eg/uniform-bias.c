/*
 # Run a few threads with jkiss pRNG, and emit those numbers modulated
 # down into the given range, presumably to be subject to various
 # statistical tests. With r-fu installed, such a test might run along
 # the lines of:
 #
 #   ./uniform-bias -c 99999 -R 99 -t 4 | r-fu equichisq -
 #
 # where a p-value of 1 is good, and a very small number, not so.
 */

#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

#include <jkiss.h>

unsigned long Flag_Count;       // -c
unsigned long Flag_Range;       // -R
unsigned long Flag_Threads;     // -c

unsigned long Threads_Completed;

pthread_mutex_t Lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t Job_Done = PTHREAD_COND_INITIALIZER;

void emit_help(void);
void *worker(void *unused);

int main(int argc, char *argv[])
{
    int ch;
    pthread_t *tids;
    struct timespec wait;

    jkiss64_init(NULL);

    setvbuf(stdout, (char *)NULL, _IOLBF, (size_t) 0);

    while ((ch = getopt(argc, argv, "h?c:R:t:")) != -1) {
        switch (ch) {

        case 'c':
            Flag_Count = flagtoul(ch, optarg, 1UL, ULONG_MAX);
            break;

        case 'R':
            Flag_Range = flagtoul(ch, optarg, 1UL, ULONG_MAX);
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

    if (Flag_Count == 0 || Flag_Range == 0 || Flag_Threads == 0)
        emit_help();

    if ((tids = calloc(sizeof(pthread_t), (size_t) Flag_Threads)) == NULL)
        err(EX_OSERR, "could not calloc() threads list");

    for (unsigned int i = 0; i < Flag_Threads; i++) {
        if (pthread_create(&tids[i], NULL, worker, NULL) != 0)
            err(EX_OSERR, "could not pthread_create() thread %d", i);
    }

    wait.tv_sec = 1;
    wait.tv_nsec = 63;

    for (;;) {
        pthread_mutex_lock(&Lock);
        if (Threads_Completed == Flag_Threads)
            break;
        pthread_cond_timedwait(&Job_Done, &Lock, &wait);
        pthread_mutex_unlock(&Lock);
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: ./uniform-bias -c count -R range -t threads\n");
    exit(EX_USAGE);
}

void *worker(void *unused)
{
    for (unsigned long i = 0; i < Flag_Count; i++)
        printf("%llu\n", jkiss64_uniform(Flag_Range));

    pthread_mutex_lock(&Lock);
    Threads_Completed++;
    pthread_mutex_unlock(&Lock);
    pthread_cond_signal(&Job_Done);

    return (void *) 0;
}
