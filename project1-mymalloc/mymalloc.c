#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

#define MEMLENGTH 4096
#define ALIGNMENT 8  
#define MIN_CHUNK_SIZE 16  


static union {
    char bytes[MEMLENGTH];
    double not_used; 
} heap;
static int initialized = 0;

//chunk implementation
struct chunk {
    size_t size;  
    int allocated; 
};


// helper function prototypes
static void initialize_heap(void);
static void leak_detection(void);

static void initialize_heap(void) {
    struct chunk *chunk = (struct chunk *) heap.bytes; //casting the heap to a chunk pointer
    chunk->size = MEMLENGTH - sizeof(struct chunk);
    chunk->allocated = 0;
    initialized = 1;
    // register leak_detection to be called at exit
    atexit(leak_detection);
}

static void leak_detection(void) {
    struct chunk *chunk = (struct chunk *)heap.bytes;
    int leak_counter = 0; 
    size_t leaked_bytes = 0;
    while((char*) chunk < heap.bytes + MEMLENGTH) {
        if(chunk->allocated){
            leak_counter ++;
            leaked_bytes += chunk->size;
        }
        chunk = (struct chunk *)((char *) chunk + sizeof(struct chunk) + chunk->size);
    }
    if(leak_counter > 0) {
        fprintf(stderr, "mymalloc: %zu bytes leaked in %d objects.\n", 
                leaked_bytes, leak_counter);
    }
}

void *mymalloc(size_t size, char *file, int line) {
    //Check initialization
    if (!initialized) {
        initialize_heap();
    }

    //Handling invalid size request
    if (size == 0) {
        fprintf(stderr, "malloc: Unable to allocate 0 bytes (%s:%d)\n", file, line);
        return NULL;
    }

    // Round up size to multiple of 8
    size_t aligned_size = (size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
    
    // Ensure we meet minimum payload size
    if (aligned_size < MIN_CHUNK_SIZE - sizeof(struct chunk)) {
        aligned_size = MIN_CHUNK_SIZE - sizeof(struct chunk);
    }

    // Calculate total chunk size needed (header + aligned payload)
    size_t total_size = sizeof(struct chunk) + aligned_size;

    //Search for a suitable free chunk
    struct chunk *chunk = (struct chunk *) heap.bytes;
    while ((char *) chunk < heap.bytes + MEMLENGTH) {
        if (!chunk->allocated && chunk->size >= aligned_size) {
            break;
        }
        chunk = (struct chunk *)((char *) chunk + sizeof(struct chunk) + chunk->size); // Move to next chunk
    }

    //If no chunk found, return NULL with error
    if ((char *) chunk >= heap.bytes + MEMLENGTH) {
        fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size, file, line);
        return NULL;
    }

    // If chunk found:
    // Split if necessary (if chunk is much bigger than needed)
    // Mark as allocated
    //Return pointer to payload area (after header)
    if (chunk->size >= aligned_size + sizeof(struct chunk) + MIN_CHUNK_SIZE) {
        struct chunk *new_chunk = (struct chunk *)((char *)chunk + sizeof(struct chunk) + aligned_size);
        new_chunk->size = chunk->size - sizeof(struct chunk) - aligned_size;
        new_chunk->allocated = 0;
        
        chunk->size = aligned_size;
        chunk->allocated = 1;
    } else {
        // If remaining space would be too small, just use the entire chunk
        chunk->allocated = 1;
    }
    
    return (void *)((char *)chunk + sizeof(struct chunk));
    }

void myfree(void *ptr, char *file, int line) {
    //Check initialization
    if (!initialized) {
        initialize_heap();
    }

    //Handle NULL pointer
    if (ptr == NULL) {
        return;
    }

    //Validate pointer
    if ((char *) ptr < heap.bytes || (char *) ptr >= heap.bytes + MEMLENGTH) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    if ((uintptr_t)ptr % 8 != 0) {
        fprintf(stderr, "free: Inappropriate pointer, misaligned (%s:%d)\n", file, line);
        exit(2);
    }

     // Get chunk header by moving backwards from payload pointer
    struct chunk *header = (struct chunk *)((char *) ptr - sizeof(struct chunk));

    //validating the header
    if ((char*)header < heap.bytes ||
    (char*)header >= heap.bytes + MEMLENGTH || 
    header->size <=0 ||
    header-> size % 8 != 0 ||
    header-> size >MEMLENGTH)
    {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    
    //Mark as free (the header of the chunk marks the whole chunk as free)
    if (!header->allocated) {
        fprintf(stderr, "free: Double free (%s:%d)\n", file, line);
        exit(2);
    }
    header->allocated = 0;


    // Coalesce with adjacent free chunks if possible
    //coallescing with the next chunk
    struct chunk *next_chunk = (struct chunk *)((char *) header + sizeof(struct chunk) + header->size);
    if ((char *) next_chunk < heap.bytes + MEMLENGTH && !next_chunk->allocated) {
        header->size += sizeof(struct chunk) + next_chunk->size;
    }
    //coallescing with the previous chunk
    struct chunk *prev_chunk = NULL;
    struct chunk *scan = (struct chunk*)heap.bytes;
    while (scan < header && scan < (struct chunk*)(heap.bytes + MEMLENGTH)) {
        if ((char*)scan + scan->size == (char*)header) {
            prev_chunk = scan;  
            break;
        }
        scan = (struct chunk*)((char*)scan + scan->size);
    }
    
    if (prev_chunk && !prev_chunk->allocated) {
        prev_chunk->size += header->size;  
    }

    
}



