#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mymalloc.h"

/**
 * Test program for error detection in mymalloc/myfree
 * 
 * This program tests the three required error conditions:
 * 1. Freeing a non-malloc address (stack variable)
 * 2. Freeing an offset pointer (not at the start of a chunk)
 * 3. Double-free detection
 * 
 * Each test is in its own function, use cmd ()./error_test # )(# number of the test you want to check for)
 */

// Test freeing a stack variable 
void test_free_non_malloc() {
    printf("\n=== Test 1: Freeing a non-malloc address (stack variable) ===\n");
    printf("Expected: This should print an error and exit\n");
    
    int x;
    printf("Attempting to free stack variable at %p\n", &x);
    free(&x);  
    
    // We should never get here
    printf("ERROR: Program did not terminate after freeing stack variable\n");
}

// Test freeing an offset pointer (not at the start of a chunk)
void test_free_offset_pointer() {
    printf("\n=== Test 2: Freeing an offset pointer ===\n");
    printf("Expected: This should print an error and exit\n");
    
    int *p = (int *)malloc(sizeof(int) * 10);
    if (p == NULL) {
        printf("Failed to allocate memory for offset pointer test\n");
        return;
    }
    
    printf("Allocated array at %p\n", p);
    printf("Attempting to free offset pointer at %p\n", p + 1);
    free(p + 1);  
    
    // We should never get here
    printf("ERROR: Program did not terminate after freeing offset pointer\n");
    free(p);  // Clean up if somehow we get here
}

// Test double-free detection
void test_double_free() {
    printf("\n=== Test 3: Double-free detection ===\n");
    printf("Expected: This should print an error and exit\n");
    
    int *p = (int *)malloc(sizeof(int) * 10);
    if (p == NULL) {
        printf("Failed to allocate memory for double-free test\n");
        return;
    }
    
    printf("Allocated memory at %p\n", p);
    printf("Freeing pointer first time (valid)\n");
    free(p);
    
    printf("Attempting to free same pointer again\n");
    free(p);  
    
    // We should never get here
    printf("ERROR: Program did not terminate after double-free\n");
}

int main(int argc, char *argv[]) {
    printf("Starting error detection tests...\n");
    printf("NOTE: This program tests error conditions that cause process termination.\n");
    printf("      Each test is run separately using command-line arguments.\n");
    
    if (argc < 2) {
        printf("\nUsage: %s <test_number>\n", argv[0]);
        printf("Test numbers:\n");
        printf("  1 - Free non-malloc address (stack variable)\n");
        printf("  2 - Free offset pointer\n");
        printf("  3 - Double-free detection\n");
        printf("  all - Run all tests (requires shell script to run each test separately)\n");
        return 0;
    }
    
    // Run specific test based on command-line argument
    if (strcmp(argv[1], "1") == 0) {
        test_free_non_malloc();
    } 
    else if (strcmp(argv[1], "2") == 0) {
        test_free_offset_pointer();
    }
    else if (strcmp(argv[1], "3") == 0) {
        test_double_free();
    }
    else if (strcmp(argv[1], "all") == 0) {
        printf("Running all tests (note: only the first will execute due to process termination)...\n");
        test_free_non_malloc();
        test_free_offset_pointer();  // This will only run if test_free_non_malloc doesn't terminate
        test_double_free();  // This will only run if previous tests don't terminate
    }
    else {
        printf("Invalid test number: %s\n", argv[1]);
        return 1;
    }
    
    printf("\nAll specified tests completed.\n");
    return 0;
}