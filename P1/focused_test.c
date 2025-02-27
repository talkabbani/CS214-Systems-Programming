/**
 * 
 * focused_test.c: memory state examination tests 
 * this test was created to debug workload 2 in memgrind 
 * due to falling into an infinite loop that was causing the program to crash (more details in readme)
 * 
 * 
 * This program focuses on examining the internal state of the heap during
 * memory allocation and deallocation operations. It provides detailed insight
 * into how the memory management system organizes and maintains chunks, which
 * is valuable for debugging and understanding the implementation.
 * 
 * Key Features:
 * - Uses the dump_heap() function to visualize heap state after operations
 * - Performs detailed testing of multiple allocations and their effects
 * - Writes specific data patterns to allocated memory to verify usability
 * - Examines heap state after both allocation and deallocation
 * 
 * Test Process:
 * 1. Allocates 5 chunks of 10 bytes each
 * 2. After each allocation:
 *    - Verifies allocation success
 *    - Writes a distinct pattern to the allocated memory
 *    - Dumps the heap state to show chunk organization
 * 
 * 3. Frees all chunks one by one
 * 4. After each deallocation:
 *    - Verifies successful freeing
 *    - Dumps the heap state to show coalescing and free chunk management
 * 
 * This test is particularly useful for visualizing how the heap evolves during
 * program execution and for verifying that the memory management implementation
 * maintains proper chunk organization and metadata integrity.
 */

#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

void dump_heap(void);

// Focus on testing the multiple allocation and freeing
void test_multiple_allocs() {
    printf("=== Testing Multiple Allocations ===\n");
    
    void *ptrs[5];
    
    printf("Allocating 5 chunks of 10 bytes each...\n");
    for (int i = 0; i < 5; i++) {
        printf("Allocating chunk %d...\n", i);
        ptrs[i] = malloc(10);
        
        if (ptrs[i] != NULL) {
            printf("  Success: Allocated chunk %d at %p\n", i, ptrs[i]);
            
            // Write to memory to verify it's usable
            char *mem = (char *)ptrs[i];
            for (int j = 0; j < 10; j++) {
                mem[j] = (char)(i + j);
            }
            printf("  Success: Wrote data to chunk %d\n", i);
        } else {
            printf("  Failed: Could not allocate chunk %d\n", i);
            break;
        }
        
        // Dump heap after each allocation
        printf("Heap state after allocation %d:\n", i);
        dump_heap();
    }
    
    printf("\nFreeing all chunks...\n");
    for (int i = 0; i < 5; i++) {
        if (ptrs[i] != NULL) {
            printf("Freeing chunk %d at %p...\n", i, ptrs[i]);
            free(ptrs[i]);
            printf("Success: Freed chunk %d\n", i);
            
            // Dump heap after each free
            printf("Heap state after freeing chunk %d:\n", i);
            dump_heap();
        }
    }
}

int main() {
    printf("Starting focused test on multiple allocations and frees...\n\n");
    
    test_multiple_allocs();
    
    printf("\nTest completed.\n");
    return 0;
}