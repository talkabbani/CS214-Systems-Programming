#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mymalloc.h"


/**
 * Test program for validating correctness properties of mymalloc/myfree
 * 
 * This program tests the following requirements:
 * 1. Memory isolation - allocated objects don't overlap
 * 2. Free memory reuse - freed memory can be allocated again
 * 3. Coalescing - adjacent free blocks are merged
 * 4. Leak detection - verify leak detector works
 */

// Test memory isolation between allocations
void test_memory_isolation() {
    printf("\n=== Testing Memory Isolation ===\n");
    printf("Allocating multiple chunks and checking for overlap\n");
    
    const int NUM_BLOCKS = 10;
    const int BLOCK_SIZE = 16;
    void *blocks[NUM_BLOCKS];
    
    // Allocate multiple blocks
    for (int i = 0; i < NUM_BLOCKS; i++) {
        blocks[i] = malloc(BLOCK_SIZE);
        if (blocks[i] == NULL) {
            printf("Failed to allocate block %d\n", i);
            for (int j = 0; j < i; j++) {
                free(blocks[j]);
            }
            return;
        }
        
        // Fill block with a unique pattern (i+1 repeated)
        memset(blocks[i], i+1, BLOCK_SIZE);
        printf("Block %d allocated at %p, filled with %d\n", i, blocks[i], i+1);
    }
    
    // Verify each block still contains its pattern
    int success = 1;
    for (int i = 0; i < NUM_BLOCKS; i++) {
        unsigned char *block = (unsigned char *)blocks[i];
        for (int j = 0; j < BLOCK_SIZE; j++) {
            if (block[j] != (unsigned char)(i+1)) {
                printf("ERROR: Block %d at %p has been corrupted at offset %d\n", i, blocks[i], j);
                printf("Expected %d, found %d\n", i+1, block[j]);
                success = 0;
                break;
            }
        }
    }
    
    if (success) {
        printf("Memory isolation test PASSED - no overlap detected\n");
    } else {
        printf("Memory isolation test FAILED - blocks are overlapping\n");
    }
    
    // Clean up
    for (int i = 0; i < NUM_BLOCKS; i++) {
        free(blocks[i]);
    }
}

// Test that freed memory can be reallocated
void test_memory_reuse() {
    printf("\n=== Testing Memory Reuse ===\n");
    
    // Allocate a chunk
    void *ptr1 = malloc(128);
    if (ptr1 == NULL) {
        printf("Failed to allocate initial block\n");
        return;
    }
    printf("Allocated block at %p\n", ptr1);
    
    // Free it
    free(ptr1);
    printf("Freed block\n");
    
    // Allocate same size again, should get same address
    void *ptr2 = malloc(128);
    if (ptr2 == NULL) {
        printf("Failed to allocate second block\n");
        return;
    }
    printf("Allocated second block at %p\n", ptr2);
    
    if (ptr1 == ptr2) {
        printf("Memory reuse test PASSED - freed memory was reused\n");
    } else {
        printf("Memory reuse test WARNING - freed memory was not reused\n");
        printf("This might be acceptable depending on implementation details\n");
        free(ptr2);
    }
}

// Test coalescing of adjacent free blocks
void test_coalescing() {
    printf("\n=== Testing Free Block Coalescing ===\n");
    
    // Step 1: Fill most of memory with small allocations
    const int ALLOC_SIZE = 24;
    const int MAX_ALLOCS = 100;  // Adjust if needed
    void *ptrs[MAX_ALLOCS];
    int count = 0;
    
    // Allocate as many blocks as possible
    printf("Step 1: Filling memory with small allocations\n");
    for (count = 0; count < MAX_ALLOCS; count++) {
        ptrs[count] = malloc(ALLOC_SIZE);
        if (ptrs[count] == NULL) break;
    }
    
    printf("Allocated %d blocks of size %d\n", count, ALLOC_SIZE);
    
    if (count < 3) {
        printf("Not enough allocations to test coalescing\n");
        for (int i = 0; i < count; i++) {
            free(ptrs[i]);
        }
        return;
    }
    
    // Step 2: Free three adjacent blocks
    printf("\nStep 2: Freeing three adjacent blocks\n");
    int middle = count / 2;
    printf("Freeing blocks at indices %d, %d, and %d\n", middle-1, middle, middle+1);
    
    free(ptrs[middle-1]);
    free(ptrs[middle]);
    free(ptrs[middle+1]);
    
    // Step 3: Try to allocate a block larger than individual blocks but smaller than combined
    int large_size = ALLOC_SIZE * 2 + sizeof(int);  // Larger than 2 blocks but smaller than 3
    printf("\nStep 3: Trying to allocate %d bytes (should fit in coalesced space)\n", large_size);
    
    void *large_ptr = malloc(large_size);
    if (large_ptr == NULL) {
        printf("Coalescing test FAILED - Could not allocate %d bytes\n", large_size);
    } else {
        printf("Allocated %d bytes at %p\n", large_size, large_ptr);
        printf("Coalescing test PASSED - Successfully allocated memory in coalesced space\n");
        free(large_ptr);
    }
    
    // Clean up remaining allocations
    for (int i = 0; i < count; i++) {
        if (i != middle-1 && i != middle && i != middle+1) {
            free(ptrs[i]);
        }
    }
}

// Intentionally leak memory to test leak detector
void test_leak_detection() {
    printf("\n=== Testing Leak Detection ===\n");
    printf("Intentionally leaking memory to test leak detector\n");
    
    const int NUM_LEAKS = 5;
    const int LEAK_SIZE = 32;
    
    for (int i = 0; i < NUM_LEAKS; i++) {
        void *ptr = malloc(LEAK_SIZE);
        if (ptr != NULL) {
            printf("Leaking %d bytes at %p\n", LEAK_SIZE, ptr);
            // Deliberately not freeing to cause leak
        }
    }
    
    printf("Leak detection test: %d chunks totaling %d bytes should be reported as leaked\n", 
           NUM_LEAKS, NUM_LEAKS * LEAK_SIZE);
    printf("Check program output at exit for leak report\n");
}

// Test alignment of returned pointers
void test_alignment() {
    printf("\n=== Testing Pointer Alignment ===\n");
    
    const int NUM_TESTS = 20;
    const int SIZES[5] = {1, 7, 13, 32, 99}; // Various sizes to test
    
    printf("Testing alignment of pointers returned by malloc\n");
    printf("All addresses should be divisible by 8 (aligned to 8 bytes)\n");
    
    int failures = 0;
    
    for (int i = 0; i < NUM_TESTS; i++) {
        int size = SIZES[i % 5];
        void *ptr = malloc(size);
        
        if (ptr == NULL) {
            printf("Failed to allocate in alignment test\n");
            continue;
        }
        
        uintptr_t addr = (uintptr_t)ptr;
        int aligned = (addr % 8 == 0);
        
        printf("Allocated %d bytes at %p - %s\n", 
               size, ptr, aligned ? "ALIGNED" : "NOT ALIGNED");
        
        if (!aligned) failures++;
        
        free(ptr);
    }
    
    if (failures == 0) {
        printf("Alignment test PASSED - All pointers properly aligned\n");
    } else {
        printf("Alignment test FAILED - %d pointers not aligned to 8 bytes\n", failures);
    }
}

int main() {
    printf("Starting validation tests for mymalloc/myfree...\n\n");
    
    // Run all validation tests
    test_memory_isolation();
    test_memory_reuse();
    test_coalescing();
    test_alignment();
    
    // Run leak test last since it intentionally leaks memory
    test_leak_detection();
    
    printf("\nAll validation tests completed.\n");
    return 0;
}