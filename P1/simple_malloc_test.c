/**
 * 
 * simple_malloc_test.c: Basic functionality tests for mymalloc/myfree
 * 
 * This program tests the basic functionality of the mymalloc/myfree implementation
 * with a series of straightforward allocations and deallocations. It focuses on
 * verifying the core functionality of the memory management system.
 * 
 * 
 * Test Cases:
 * 1. Basic malloc/free: Simple allocation and deallocation of single objects
 *    - Small allocation (1 byte)
 *    - Larger allocation (100 bytes)
 * 
 * 2. Multiple allocations: Allocate and free multiple chunks
 *    - 5 chunks of 10 bytes each
 *    - Verifies all allocations succeed and can be freed
 * 
 * 3. Sequential allocations: Allocate blocks of different sizes in sequence
 *    - Allocates 10 blocks with increasing sizes
 *    - Tests memory utilization with varying allocation sizes
 * 
 * 4. Coalescing: Tests that adjacent free blocks are merged
 *    - Allocates 3 blocks in a row
 *    - Frees them in a specific order to test coalescing
 *    - Attempts to allocate in the coalesced space
 * 
 * This test program focuses on basic functionality rather than error detection,
 *  to verify that the memory management system works
 * correctly under normal usage patterns.
 */

#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

void test_basic_malloc_free() {
    printf("=== Testing Basic Malloc/Free ===\n");
    
    printf("Test 1: Allocate and free 1 byte\n");
    void *ptr = malloc(1);
    if (ptr != NULL) {
        printf("  Success: Allocated 1 byte at %p\n", ptr);
        free(ptr);
        printf("  Success: Freed 1 byte\n");
    } else {
        printf("  Failed: Could not allocate 1 byte\n");
    }
    
    printf("\nTest 2: Allocate and free 100 bytes\n");
    ptr = malloc(100);
    if (ptr != NULL) {
        printf("  Success: Allocated 100 bytes at %p\n", ptr);
        free(ptr);
        printf("  Success: Freed 100 bytes\n");
    } else {
        printf("  Failed: Could not allocate 100 bytes\n");
    }
    
    printf("\nTest 3: Allocate 5 chunks of 10 bytes each\n");
    void *ptrs[5];
    int success = 1;
    
    for (int i = 0; i < 5; i++) {
        ptrs[i] = malloc(10);
        if (ptrs[i] != NULL) {
            printf("  Success: Allocated chunk %d at %p\n", i, ptrs[i]);
        } else {
            printf("  Failed: Could not allocate chunk %d\n", i);
            success = 0;
            break;
        }
    }
    
    if (success) {
        printf("  Freeing all 5 chunks\n");
        for (int i = 0; i < 5; i++) {
            free(ptrs[i]);
            printf("  Success: Freed chunk %d\n", i);
        }
    }
    
    printf("\nTest 4: Allocate after free\n");
    ptr = malloc(50);
    if (ptr != NULL) {
        printf("  Success: Allocated 50 bytes at %p\n", ptr);
        free(ptr);
        printf("  Success: Freed 50 bytes\n");
        
        ptr = malloc(50);
        if (ptr != NULL) {
            printf("  Success: Re-allocated 50 bytes at %p\n", ptr);
            free(ptr);
            printf("  Success: Freed 50 bytes again\n");
        } else {
            printf("  Failed: Could not re-allocate 50 bytes\n");
        }
    } else {
        printf("  Failed: Could not allocate 50 bytes\n");
    }
}

void test_sequential_alloc() {
    printf("\n=== Testing Sequential Allocations ===\n");
    
    const int NUM_ALLOCS = 10;
    void *ptrs[NUM_ALLOCS];
    
    // Allocate blocks in sequence
    for (int i = 0; i < NUM_ALLOCS; i++) {
        ptrs[i] = malloc(8 * (i + 1)); // Different sizes
        if (ptrs[i] != NULL) {
            printf("Allocated %d bytes at %p\n", 8 * (i + 1), ptrs[i]);
            
            // Write to memory to verify it's usable
            char *mem = (char *)ptrs[i];
            for (int j = 0; j < 8 * (i + 1); j++) {
                mem[j] = (char)(j % 256);
            }
        } else {
            printf("Failed to allocate %d bytes\n", 8 * (i + 1));
            break;
        }
    }
    
    // Free blocks in sequence
    for (int i = 0; i < NUM_ALLOCS; i++) {
        if (ptrs[i] != NULL) {
            free(ptrs[i]);
            printf("Freed allocation at %p\n", ptrs[i]);
        }
    }
}

// Test coalescing of free blocks
void test_coalescing() {
    printf("\n=== Testing Free Block Coalescing ===\n");
    
    // Allocating 3 blocks in a row
    void *p1 = malloc(100);
    void *p2 = malloc(100);
    void *p3 = malloc(100);
    
    printf("Allocated 3 blocks: %p, %p, %p\n", p1, p2, p3);
    
    // Free middle block, should not coalesce
    free(p2);
    printf("Freed middle block at %p\n", p2);
    
    // Free first block, should coalesce with middle
    free(p1);
    printf("Freed first block at %p (should coalesce with middle)\n", p1);
    
    // Allocate a block that should fit in coalesced space
    void *p4 = malloc(150);
    printf("Allocated 150 bytes at %p (should use coalesced space)\n", p4);
    
    // Free remaining blocks
    free(p3);
    printf("Freed block at %p\n", p3);
    if (p4 != NULL) {
        free(p4);
        printf("Freed block at %p\n", p4);
    }
}

int main() {
    printf("Starting simple malloc/free tests...\n\n");
    
    test_basic_malloc_free();
    test_sequential_alloc();
    test_coalescing();
    
    printf("\nAll tests completed.\n");
    return 0;
}