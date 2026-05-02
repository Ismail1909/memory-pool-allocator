CC= g++
DEBUG = -g -Og
RELEASE = -O2
CFLAGS= -c -Wall -m32 $(DEBUG)
LINKER= g++
LINK= -m32


all: allocator

clean:
	rm -rf allocator allocator.o heap.o

allocator: allocator.o heap.o
	$(LINKER) $(LINK) -o $@ $^

heap.o: heap.asm
	nasm -f elf $^

allocator.o: alloc.cpp
	$(CC) $(CFLAGS) $^ -o $@