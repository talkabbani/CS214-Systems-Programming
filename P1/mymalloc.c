/**
 * 
 * mymalloc.c: Implementation of custom malloc/free with error detection
 * 
 * This file implements custom versions of the standard library functions
 * malloc() and free() that can detect and report common usage errors.
 * 
 * Memory Management Features:
 * - Uses a static array of 4096 bytes to simulate the heap
 * - Manages memory as chunks with headers containing metadata
 * - Ensures 8-byte alignment for all allocations
 * - Coalesces adjacent free blocks to reduce fragmentation
 * - Splits large free blocks to minimize wasted space
 * 
 * Error Detection Capabilities:
 * 1. Out-of-memory conditions: Reports when allocation requests can't be fulfilled
 * 2. Invalid pointers: Detects when free() is called with addresses not from malloc()
 * 3. Misaligned pointers: Detects when free() is called with addresses not at chunk starts
 * 4. Double-free: Detects when free() is called on already freed memory
 * 5. Memory leaks: Reports any unfreed allocations at program termination
 * 
 * When an error is detected, the implementation prints a descriptive error message
 * that includes the source file and line number where the error occurred, then
 * terminates the program with exit code 2.
 * 
 * Heap initialization and leak detection are handled automatically, with no need
 * for explicit initialization by client code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "mymalloc.h"
#include <stdarg.h>

#ifndef MEMLENGTH
#define MEMLENGTH 4096
#endif

#ifndef ALIGNMENT
#define ALIGNMENT 8  
#endif 

#ifndef MIN_CHUNK_SIZE 
#define MIN_CHUNK_SIZE 16 
#endif 
 
#ifndef DEBUG
#define DEBUG 0  
#endif 


// Chunk structure
typedef struct chunk {
    size_t size;      // Size of the payload area
    int allocated;    // 1 if allocated, 0 if free
} chunk_t;

static union {
    char bytes[MEMLENGTH];
    double not_used; 
} heap;

static int initialized = 0;

// Debug function to print messages if DEBUG is enabled
void debug_print(const char* format, ...) {
    #if DEBUG
    va_list args;
    va_start(args, format);
    fprintf(stderr, "[DEBUG] ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
    #endif
}

// Helper function prototypes
static void initialize_heap(void);
static void leak_detection(void);

// Initialize the heap with a single free chunk
static void initialize_heap(void) {
    debug_print("Initializing heap");
    chunk_t *init_chunk = (chunk_t *)heap.bytes;
    init_chunk->size = MEMLENGTH - sizeof(chunk_t);
    init_chunk->allocated = 0;
    initialized = 1;
    
    // Register leak detection to run at program exit
    atexit(leak_detection);
    debug_print("Heap initialized with a free chunk of size %zu bytes", init_chunk->size);
}

// Scan for leaks at program termination
static void leak_detection(void) {
    int leak_count = 0;
    size_t leaked_bytes = 0;
    
    debug_print("Running leak detection");
    
    chunk_t* current = (chunk_t*)heap.bytes;
    
    while ((char*)current < heap.bytes + MEMLENGTH) {
        if (current->allocated) {
            leak_count++;
            leaked_bytes += current->size;
            debug_print("Found leaked chunk at %p, size %zu", current, current->size);
        }
        
        // Move to next chunk
        current = (chunk_t*)((char*)current + sizeof(chunk_t) + current->size);
        
        // Break if we've reached or exceeded the end of the heap
        if ((char*)current >= heap.bytes + MEMLENGTH) {
            break;
        }
    }
    
    if (leak_count > 0) {
        fprintf(stderr, "mymalloc: %zu bytes leaked in %d objects.\n", 
                leaked_bytes, leak_count);
    } else {
        debug_print("No memory leaks detected");
    }
}

// Function to dump heap state - helps with debugging
void dump_heap() {
    printf("\n=== HEAP DUMP ===\n");
    chunk_t* current = (chunk_t*)heap.bytes;
    int count = 0;
    
    while ((char*)current < heap.bytes + MEMLENGTH) {
        printf("Chunk %d: addr=%p, size=%zu, allocated=%d, payload_addr=%p\n", 
               count++, current, current->size, current->allocated, 
               (void*)((char*)current + sizeof(chunk_t)));
        
        // Move to next chunk
        current = (chunk_t*)((char*)current + sizeof(chunk_t) + current->size);
        
        // Break if we've reached or exceeded the end of the heap
        if ((char*)current >= heap.bytes + MEMLENGTH) {
            break;
        }
    }
    printf("=== END HEAP DUMP ===\n\n");
}

void *mymalloc(size_t size, char *file, int line) {
    // Initialize heap if needed
    if (!initialized) {
        initialize_heap();
    }
    
    debug_print("mymalloc(%zu) called from %s:%d", size, file, line);
    
    // Handle invalid size
    if (size == 0) {
        fprintf(stderr, "malloc: Unable to allocate 0 bytes (%s:%d)\n", file, line);
        return NULL;
    }
    
    // Round up size to multiple of ALIGNMENT
    size_t aligned_size = (size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
    debug_print("Aligned size: %zu bytes", aligned_size);
    
    // Ensure we meet minimum payload size
    if (aligned_size < MIN_CHUNK_SIZE - sizeof(chunk_t)) {
        aligned_size = MIN_CHUNK_SIZE - sizeof(chunk_t);
        debug_print("Adjusted to minimum size: %zu bytes", aligned_size);
    }
    
    // Find a suitable free chunk
    chunk_t* current = (chunk_t*)heap.bytes;
    debug_print("Starting search for free chunk");
    
    while ((char*)current < heap.bytes + MEMLENGTH) {
        debug_print("Examining chunk at %p, size: %zu, allocated: %d", 
                   current, current->size, current->allocated);
                   
        if (!current->allocated && current->size >= aligned_size) {
            debug_print("Found suitable free chunk at %p with size %zu", current, current->size);
            
            // Split the chunk if it's significantly larger than what we need
            if (current->size >= aligned_size + sizeof(chunk_t) + MIN_CHUNK_SIZE) {
                chunk_t* new_chunk = (chunk_t*)((char*)current + sizeof(chunk_t) + aligned_size);
                size_t remaining_size = current->size - aligned_size - sizeof(chunk_t);
                
                debug_print("Splitting chunk. New free chunk at %p with size %zu", 
                           new_chunk, remaining_size);
                
                new_chunk->size = remaining_size;
                new_chunk->allocated = 0;
                current->size = aligned_size;
            }
            
            // Mark as allocated and return pointer to payload
            current->allocated = 1;
            void* payload = (void*)((char*)current + sizeof(chunk_t));
            debug_print("Returning payload pointer %p", payload);
            return payload;
        }
        
        // Move to next chunk
        chunk_t* next = (chunk_t*)((char*)current + sizeof(chunk_t) + current->size);
        
        // Safety check to avoid infinite loops(since the first attempt implemntation had this issue)
        if (next <= current || (char*)next >= heap.bytes + MEMLENGTH) {
            debug_print("Warning: Invalid next chunk pointer. Breaking search loop.");
            break;
        }
        
        current = next;
    }
    
    // No suitable chunk found
    debug_print("No suitable free chunk found");
    fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size, file, line);
    return NULL;
}

void myfree(void *ptr, char *file, int line) {
    debug_print("myfree(%p) called from %s:%d", ptr, file, line);
    
    // Handle nulll pointer
    if (ptr == NULL) {
        debug_print("NULL pointer passed to free - nothing to do");
        return;
    }
    
    // Check if pointer is within heap bounds
    if ((char*)ptr < heap.bytes || (char*)ptr >= heap.bytes + MEMLENGTH) {
        fprintf(stderr, "free: Inappropriate pointer, out of bounds (%s:%d)\n", file, line);
        exit(2);
    }
    
    // check alignment
    if ((uintptr_t)ptr % ALIGNMENT != 0) {
        fprintf(stderr, "free: Inappropriate pointer, misaligned (%s:%d)\n", file, line);
        exit(2);
    }
    
    // Get chunk header from payload pointer
    chunk_t* chunk = (chunk_t*)((char*)ptr - sizeof(chunk_t));
    debug_print("Chunk header at %p, size: %zu, allocated: %d", 
               chunk, chunk->size, chunk->allocated);
    
    // Validate chunk
    if ((char*)chunk < heap.bytes || 
        (char*)chunk + sizeof(chunk_t) + chunk->size > heap.bytes + MEMLENGTH ||
        chunk->size == 0 || 
        chunk->size % ALIGNMENT != 0) {
        fprintf(stderr, "free: Inappropriate pointer, invalid chunk header (%s:%d)\n", file, line);
        exit(2);
    }
    
    // Check if already freed (double free)
    if (!chunk->allocated) {
        fprintf(stderr, "free: Double free (%s:%d)\n", file, line);
        exit(2);
    }
    
    // Mark as free
    chunk->allocated = 0;
    debug_print("Chunk marked as free");
    
    // Try to coalesce with next chunk if it's free
    chunk_t* next = (chunk_t*)((char*)chunk + sizeof(chunk_t) + chunk->size);
    if ((char*)next < heap.bytes + MEMLENGTH) {
        debug_print("Next chunk at %p, size: %zu, allocated: %d", 
                   next, next->size, next->allocated);
                   
        if (!next->allocated) {
            debug_print("Coalescing with next chunk (size: %zu)", next->size);
            chunk->size += sizeof(chunk_t) + next->size;
            debug_print("New size after forward coalescing: %zu", chunk->size);
        }
    }
    
    // coalesce with previous chunk if it's free
    // We need to scan from the beginning since we can't go backward easily
    chunk_t* prev = NULL;
    chunk_t* scan = (chunk_t*)heap.bytes;
    
    debug_print("Scanning for previous chunk to coalesce");
    
    while (scan != chunk && (char*)scan < heap.bytes + MEMLENGTH) {
        chunk_t* next_scan = (chunk_t*)((char*)scan + sizeof(chunk_t) + scan->size);
        
        if (next_scan == chunk) {
            prev = scan;
            debug_print("Found previous chunk at %p, size: %zu, allocated: %d", 
                       prev, prev->size, prev->allocated);
            break;
        }
        
        // Safety check to avoid infinite loops
        if (next_scan <= scan || (char*)next_scan >= heap.bytes + MEMLENGTH) {
            debug_print("Warning: Invalid scan pointer. Breaking prev search loop.");
            break;
        }
        
        scan = next_scan;
    }
    
    if (prev != NULL && !prev->allocated) {
        debug_print("Coalescing with previous chunk (size: %zu)", prev->size);
        prev->size += sizeof(chunk_t) + chunk->size;
        debug_print("New size after backward coalescing: %zu", prev->size);
    }
    
    debug_print("Free operation completed successfully");
}


