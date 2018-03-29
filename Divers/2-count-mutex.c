#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

// total count
long gCount = 0;

// exlusive lock for total count
pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;

void usage(char* iName) {
    printf("\nUsage> %s number_of_threads per_thread_count\n", iName);
    exit(-1);
}

void* count(void* iArg) {
    long lMaxCount = (long)iArg;
    for(int i=0; i<lMaxCount; ++i) {
        pthread_mutex_lock(&gMutex);
        ++gCount;
        pthread_mutex_unlock(&gMutex);
    }
    pthread_exit(NULL);
}

int main(int iArgc, char** iArgv) {
    if(iArgc < 2 || iArgc > 3) usage(iArgv[0]);
    int lThreads = atoi(iArgv[1]);
    long lMaxCount = 1e6;
    if(iArgc == 3) lMaxCount = atol(iArgv[2]);
    printf("\nProblem is %d threads of %ld = %ld\n", lThreads, 
            lMaxCount, lThreads*lMaxCount);

    // create threads
    pthread_t lIds[lThreads];
    printf("Starting threads");
    for(int i=1; i<=lThreads; ++i) {
        printf(" %d", i);
        pthread_create(&lIds[i-1], NULL, count, (void*)lMaxCount);
    }

    // wait for thread completion
    printf("\nJoining threads ");
    for(int i=1; i<=lThreads; ++i) {
        pthread_join(lIds[i-1], NULL);
        printf(" %d", i);
    }
    printf("\n\nFinal count is %ld (error=%ld)\n", gCount,
           lThreads*lMaxCount - gCount);
}