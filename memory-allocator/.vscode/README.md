# Memory allocator from scratch in C++

# Build environment

- Target: 32-bit elf build.
- Environment:  WSL Linux x86_64.

## Prerequisites
- GNU GCC / G++ compiler
- Make
- NASM assembler

# Project structure
- `alloc.cpp/alloc.h` - The implementation of the memory allocator.
- `main.cpp` - The main program that tests the allocator.
- `heap.asm` - The assembly file that reserves block of memory for our allocator. The purpose of this file is to create a large block of memory that our allocator can manage. This block will be used to fulfill allocation requests from the main program.

# Project journey

## Project setup & Heap reservation
1. We created `heap.asm` to reserve a block of memory for our allocator. This file contains assembly code that defines a large block of memory (e.g., 1 MB) that our allocator can manage. The block is defined using the `section .heap` directive, which reserves uninitialized space in memory.

    ```asm
    bits 32 ; This is to specify that we are working with 32-bit code.
    global memspace ; mark symbol as global so it can be seen to other obj files during linkage.

    %define HeapSize (1024*1024*1024/4) ; preprocessing definition
    ```

    Section definition:

    section <name> [attributes], All info about section attributes [here](https://www.tortall.net/projects/yasm/manual/html/objfmt-elf-section.html)

    ```asm
    section .heap noexec nobits write align=4
    ```

    Reserving memory:

    ```asm
    section .heap noexec nobits write align=4
        __memspace: ; // to define symbolic address
            heapSize equ HeapSize ;  asm named constant assignment so the constant value now has label 'heapSize'
            resd heapSize ; pseudo-instruction to declare & reserve uninitialized memory.
            ; res(no of bytes): resb - one byte, resw - 2 bytes, resd - 4 bytes
            ; if nobits, the memory is allocated and initialized at load.
            ; if progbits, the memory is reserved in the disk image.
    ```

    Storing address of allocated memory in the global symbol.

    ```asm
    section .data
    memspace:
        dd __memspace ; store memory address in the symbol 'memspace'.
    ```

2. The general idea is that we are going to allocate memory in words, each allocation takes a header of size one word(32-bits) followed by the requested memory in words.

    Example: For 2 word (64-bits) memory requested
    ```
    memory block(e.g 1GB)
    |32-bit header|memory section|memory section| | | | | | | | | | |.....
    ```

    The header struct is defined as follows:

    ```cpp
    typedef uint32 word;
    struct packed s_header {
        word m_word:30; // number of words allocated
        bool m_isAllocated:1;
        bool unused m_reserved:1;
    };
    ```

    We declare `memspace` pointer so we have reference to the address of block of memory allocated from the assembly file.

    ```cpp
    extern void* memspace;
    ```

## Initial Allocation function:

The logic for allocating memory goes as follows:

The function `alloc` takes in the number of bytes requested and returns a pointer to the allocated memory or `NULL` if the allocation fails and sets `errno` to `ENOMEM`.

1. First check if the requested size is zero, if it is we set `errno` to `EINVAL` and return `NULL`.

2. Calculate the number of words needed to fulfill the request. We round up the requested size to the nearest word boundary.

3. Check if the requested number of words exceeds the total number of words available in the heap. If it does, we set `errno` to `ENOMEM` and return `NULL`.

4. Create a header pointer to the start of the heap and pass it down to `mkAlloc` along with the number of words needed.

5. The `mkAlloc` does the actual allocation if available, it checks if the no of words requested exceeds the available memory and if so return `NULL`.

6. If not it checks if the header was set before, checking if it is still marked as allocated & the number of words allocated, If not allocated, that means there is a free block of memory that can be reused, but it checks if the free block is large enough to accommodate the requested number of words. If it's then fill the header and return the pointer to the allocated memory. If not, it returns `NULL`.

7. However if the header was not allocated before meaning(header->word == 0), then it fills the header with the number of words allocated and marks it as allocated, then returns the pointer to the allocated memory.
