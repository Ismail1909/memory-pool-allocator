CC= g++
CFLAGS= -c -Wall -O2 -m32 -g
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