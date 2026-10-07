CC = gcc
CFLAGS = -Wall

all: parallel_min_max process_memory psum

parallel_min_max: src/parallel_min_max.c
	$(CC) $(CFLAGS) -o $@ $<

process_memory: src/process_memory.c
	$(CC) $(CFLAGS) -o $@ $<

psum_lib.o: src/psum_lib.c src/psum_lib.h
	$(CC) $(CFLAGS) -c src/psum_lib.c -o psum_lib.o

libpsum.a: psum_lib.o
	ar rcs libpsum.a psum_lib.o

psum: src/parallel_sum.c libpsum.a
	$(CC) $(CFLAGS) -o $@ src/parallel_sum.c -L. -lpsum -lpthread

clean:
	rm -f parallel_min_max process_memory psum psum_lib.o libpsum.a
