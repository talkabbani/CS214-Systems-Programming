# Project 1: mymalloc()
**By: Tala Alkabbani (ta671) & Kamel AbuAlyaqien (kfa35)**

# Contents: 
1. Introduction 
2. Design Plan 
3. Implemntation 
4. Correctness Testing
5. Stress Testing
6. How to Test 



## 1. Introduction
mymalloc, is a a custom memory allocation library that is goal is to deepen our understanding of how memory is allocated and managed during the program's execution. 
In this project we have implemented our version of the standard C library function `malloc()` and `free()`.  Unlike the standard implementations, our version detecs common usage errors and report them. 
The standard `malloc()` and `free()` provide basic memory managment yet offer limited error detction capabilities.
This implemntation detects issues like trying to free memory that wasn't allocated with malloc,  freeing memory that was already freed (double freeing) and freeing addresses that don't points to the begging of an allocated chunk.

## 2. Design Plan
The memory allocation libraray models the heap as a sequence of variably sized chunks, where each chunk is a contiguous sequence of bytes. We use a 4096 byte static array to simulate the heap memory space, which is divided entirely into chunks. Each chunk has two components: a header conatining **metadata** about the chunk *size and allocation status* and a **payload** area where *the user's data is stored*.

The allocator is designed to maintain proper 8-byte alignment for all memory addresses, which ensures compatibility with most data types that have specific alignment requirements. When a user requests memory, it searches for a suitable free chunk, splits it if needed, and returns a pointer to the payload area.

The free operation marks memory as available and it also implements coalescing, where adjacent free chunks are automatically combined into larger chunks to prevent memory fragmentation.  

## 3. Implementation

The  implementation of  the chunk structure contains information about the size of the memory block and whether it's currently allocated or free. We maintain these chunks in a way that allows us to traverse the entire heap by adding each chunk's size to its address to find the next chunk. 

We've implemented automatic heap initialization that occurs on the first call to `mymalloc()` or `myfree()`. This creates a single large free chunk spanning the entire heap. We also register a leak detection function using `atexit()` during initialization, which runs when the program terminates to report any memory that wasn't properly free.


## 4. Testing and Debugging Process

During development, when running our initial test cases for allocating multiple memory chunks and then freeing them, the program would successfully allocate all chunks but either hang or crash during the free operations. 

To resolve these issues, we first implemented a program tht tests the basic functionality of the `malloc()` and `free()`: simple_malloc_test.c and focused_test.c  
focused_test.c file allocates 5 chunks of 10 bytes each, writes unique data to each chunk, and dumpes the heap state after each operation, which provided a visual inspection of chunk headers, sizes, and allocation status.

Also, to tackle the issue we added detailed debugging messages in mymalloc.c so that it's enbled with a debugging flag, showing exactly what was happening during each allocation and free operation. 

 This is where we were able to pin point that the issue was the way we were handling the chunk navigation in **mymalloc.c**. The methods for traversing chunks were inconsistent, which sometimes led to infinite loops.

 We fixed it by standardizing chunk navigation using consistent pointer arithmetic. The coalescing logic had flaws as well, particularly in how we scanned for previous chunks.
 Our first attempt to code this the code didn't correctly account for the chunk header size when calculating the next chunk position, resulting in incorrect coalescing. We wrote a dedicated function for traversal that helps us get the next chunk correctly.


## 5. Correctness Testing
To verify that our memory allocator meets all requirements, we've developed a suite of test programs that examine different aspects of its behavior:
The **simple_malloc_test.c** program tests the fundamental operations of our allocator. It allocates and frees memory of various sizes, checks that freed memory can be reallocated, and verifies that our allocator can handle a sequence of allocation requests. This program helped us confirm that the basic functionality was working correctly before we moved on to more complex testing.


Our **validation_test.c** verifies memory isolation by allocating multiple blocks, filling each with a unique pattern, and checking that the patterns remain intact—confirming that writing to one block doesn't affect others. It also tests memory reuse by allocating a block, freeing it, and then allocating again to see if the same memory address is returned. Additionally, it checks that coalescing works correctly by freeing adjacent blocks and then trying to allocate a larger block that should fit in the combined space. The program also verifies that returned pointers are properly aligned to 8-byte. 


**the error_test.c** program is for error detecting. It attempts to free a stack variable that is not allocated with malloc, free a pointer that points to the middle of an allocated block, and free the same memory twice. These tests confirm that our error detection mechanism works as expected.


## 6. Stress Testing

In **memgrind.c** we performed 5 different workloads that show how our allocator handles large numbers of memory operations.  It runs five different workloads 50 times each and reports the average execution time for each workload

**Workload 1** (Given in the project requiremnts)
This test repeatedly allocates and immediately frees a single byte, 120 times in a row. 

**Workload 2** (Given in the project requiremnts)
 Allocates 120 single-byte objects, storing all the pointers in an array, and then frees them all.


**Workload 3** (Given in the project requiremnts)
Makes random choices between allocating new memory and freeing previously allocated memory, until 120 allocations have been performed in total

**Workload 4** (Our own test)
 Creates and destroys a linked list with 120 nodes. It allocates memory for each node, links them together, and then traverses the list to free all nodes

**Workload 5** (Our own test)
Works with a dynamic 2D array, allocating a 15x8 matrix of integers. It first allocates an array of row pointers, then allocates memory for each row, initializes the array with values, and finally frees each row and the array of pointers. 

## 7. How to Test

To test our implementation, we've wrote a bash script called run_tests.sh that runs all the test programs in sequence. To use it:

Compile all files using make
Run the test script: ./run_tests.sh

The script will run the validation tests, error detection tests (each in a separate process since they cause termination), and the memgrind performance tests. Each test will output its results to the console.
For individual testing:

**./validation_test**   *Run correctness validation*

**./error_test 1**      *Test freeing non-malloc address*

**./error_test 2**      *Test freeing offset pointer*

**./error_test 3**      *Test double-free detection*

**./memgrind**          *Run performance tests*
