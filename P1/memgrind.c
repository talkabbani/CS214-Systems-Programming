/**
 *
 * memgrind.c: Performance testing program for mymalloc/myfree
 * 
 * This program performs stress testing on the mymalloc/myfree implementation
 * by running five different workloads 50 times each and reporting the average
 * execution time. The workloads are designed to simulate different memory 
 * allocation patterns that might be encountered in real applications.
 * 
 * Workloads:
 * 1. Sequential malloc/free: Allocate and immediately free 1 byte, 120 times
 * 2. Batch allocation: Allocate 120 1-byte objects, then free all
 * 3. Random allocation/deallocation: Randomly choose between allocating a new
 *    1-byte object or freeing a previously allocated one, until 120 allocations
 * 4. Linked list: Create and destroy a linked list with 120 nodes
 * 5. Dynamic 2D array: Allocate and free a 2D array
 * 
 * Each workload is run 50 times, and the average execution time in microseconds
 * is reported at the end.
 */

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

// Workload 4: Linked list - Create/destroy a linked list
typedef struct node {
    int data;
    struct node *next;
} Node;

void test_workload4() {
    Node *head = NULL;
    
    // Create a linked list with 120 nodes
    for (int i = 0; i < 120; i++) {
        // Create a new node
        Node *newNode = (Node*)malloc(sizeof(Node));
        newNode->data = i;
        newNode->next = head;
        head = newNode;
    }
    
    // Traverse and free the linked list
    while (head != NULL) {
        Node *temp = head;
        head = head->next;
        free(temp);
    }
}

// Workload 5: Dynamic 2D array - Allocate and free a 2D array
void test_workload5() {
    int rows = 15;
    int cols = 8;
    
    // Allocate array of row pointers
    int **matrix = (int**)malloc(rows * sizeof(int*));
    
    // Allocate each row
    for (int i = 0; i < rows; i++) {
        matrix[i] = (int*)malloc(cols * sizeof(int));
        
        // Initialize with some values
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = i * cols + j;
        }
    }
    
    // Free the matrix
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

int main() {
    struct timeval start, end;
    long total_times[5] = {0}; // Array to track time for each workload
    
    // Initialize random seed
    srand(time(NULL));
    
    printf("Running memgrind performance tests...\n");
    
    for (int i = 0; i < 50; i++) {
        printf("Run %d/50\n", i+1);
        
        // Workload 1
        gettimeofday(&start, NULL);
        test_workload1();
        gettimeofday(&end, NULL);
        total_times[0] += (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
        
        // Workload 2
        gettimeofday(&start, NULL);
        test_workload2();
        gettimeofday(&end, NULL);
        total_times[1] += (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
        
        // Workload 3
        gettimeofday(&start, NULL);
        test_workload3();
        gettimeofday(&end, NULL);
        total_times[2] += (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
        
        // Workload 4
        gettimeofday(&start, NULL);
        test_workload4();
        gettimeofday(&end, NULL);
        total_times[3] += (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
        
        // Workload 5
        gettimeofday(&start, NULL);
        test_workload5();
        gettimeofday(&end, NULL);
        total_times[4] += (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    }
    
    printf("\nResults:\n");
    printf("Workload 1 (Malloc/free in sequence): Average time %f microseconds\n", total_times[0] / 50.0);
    printf("Workload 2 (Malloc all, then free all): Average time %f microseconds\n", total_times[1] / 50.0);
    printf("Workload 3 (Random malloc/free): Average time %f microseconds\n", total_times[2] / 50.0);
    printf("Workload 4 (Linked list): Average time %f microseconds\n", total_times[3] / 50.0);
    printf("Workload 5 (Dynamic 2D array): Average time %f microseconds\n", total_times[4] / 50.0);
    
    double total_average = (total_times[0] + total_times[1] + total_times[2] + 
                            total_times[3] + total_times[4]) / 50.0 / 5.0;
    printf("\nOverall average time across all workloads: %f microseconds\n", total_average);
    
    return 0;
}