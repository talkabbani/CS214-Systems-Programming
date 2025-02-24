#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"
#include <sys/time.h>
#include <time.h>

// Workload 1: Malloc/free 1 byte 120 times
void test_workload1() {
    for (int i = 0; i < 120; i++) {
        void *ptr = malloc(1);
        free(ptr);
    }
}

// Workload 2: Malloc 120 1-byte chunks, then free all
void test_workload2() {
    void *ptrs[120];
    for (int i = 0; i < 120; i++) {
        ptrs[i] = malloc(1);
    }
    for (int i = 0; i < 120; i++) {
        free(ptrs[i]);
    }
}

// Workload 3: Random malloc/free until 120 allocations
void test_workload3() {
    void *ptrs[120] = {NULL};
    int allocated = 0;
    int inUse = 0;
    
    while (allocated < 120) {
        if (inUse == 0 || (rand() % 2 == 0 && allocated < 120)) {
            // Allocate
            ptrs[inUse++] = malloc(1);
            allocated++;
        } else {
            // Free
            int index = rand() % inUse;
            free(ptrs[index]);
            ptrs[index] = ptrs[--inUse];
        }
    }
    
    // Free remaining blocks
    while (inUse > 0) {
        free(ptrs[--inUse]);
    }
}

int main() {
    struct timeval start, end;
    long total_time = 0;
    
    for (int i = 0; i < 50; i++) {
        gettimeofday(&start, NULL);
        
        test_workload1();
        test_workload2();
        test_workload3();
        // Add workloads 4 and 5
        
        gettimeofday(&end, NULL);
        total_time += (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    }
    
    printf("Average time per run: %f microseconds\n", total_time / 50.0);
    return 0;
}