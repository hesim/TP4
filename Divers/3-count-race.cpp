#include <iostream>
#include <vector>
#include <thread>

// total count
long gCount = 0;

void usage(char* iName) {
    printf("\nUsage> %s number_of_threads per_thread_count\n", iName);
    exit(-1);
}

void count(long iMaxCount) {
    for(int i=0; i<iMaxCount; ++i) {
        ++gCount;
    }
}

int main(int iArgc, char** iArgv) {
    if(iArgc < 2 || iArgc > 3) usage(iArgv[0]);
    int lThreads = atoi(iArgv[1]);
    long lMaxCount = 1e6;
    if(iArgc == 3) lMaxCount = atol(iArgv[2]);
    printf("\nProblem is %d threads of %ld = %ld\n", lThreads, 
            lMaxCount, lThreads*lMaxCount);

    // create threads
    std::vector<std::thread> lIds;
    printf("Starting threads");
    for(int i=1; i<=lThreads; ++i) {
        printf(" %d", i);
        lIds.push_back(std::thread(count, lMaxCount));
    }

    // wait for thread completion
    printf("\nJoining threads ");
    for(int i=1; i<=lThreads; ++i) {
        lIds[i-1].join();
        printf(" %d", i);
    }
    printf("\n\nFinal count is %ld (error=%ld)\n", gCount,
           lThreads*lMaxCount - gCount);
}