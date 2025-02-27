CC = gcc
CFLAGS = -g -Wall -Werror
DEPS = mymalloc.h

TARGETS = memgrind simple_malloc_test focused_test error_test validation_test

all: $(TARGETS)

memgrind: memgrind.o mymalloc.o
	$(CC) $(CFLAGS) -o $@ $^

simple_malloc_test: simple_malloc_test.o mymalloc.o
	$(CC) $(CFLAGS) -o $@ $^

focused_test: focused_test.o mymalloc.o
	$(CC) $(CFLAGS) -o $@ $^

error_test: error_test.o mymalloc.o
	$(CC) $(CFLAGS) -o $@ $^

validation_test: validation_test.o mymalloc.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(TARGETS)

run-tests: all
	chmod +x run_tests.sh
	./run_tests.sh