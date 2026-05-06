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


## Free function:

The `destroy` works as the standard `free` function, it takes in a pointer to the memory to be freed and returns nothing. It checks if the pointer is `NULL`, if it is, it does nothing. Otherwise, it gets the header and marks the block as free by setting the `m_isAllocated` flag to `false` & wipes the memory block. but if the isAllocated flag is already false, that means the block is already free and it triggers abort to prevent double free.

## Support multiple allocation with find next free block:

To support multiple allocations, we need to find the next free block of memory after the current allocation. This is done by iterating through the heap and checking the headers of each block until we find a free block that is large enough to accommodate the requested number of words.

The logic goes as follows: we check the start position of the heap and check the header, if if no of words is zero, that means the block is unallocated and we can use it, otherwise, it means the block is either allocated or marked as free, if `isAllocated` is false, that means the block is free and we can reuse it if it is large enough to accomodate the requested number of words, if not then we move to the next block by adding the size of the current block (header + allocated words) to the current position and repeat the process until we find a suitable block or reach the end of the heap.

However the logic has a flaw, if some blocks are freed & reused, they can overlap with next allocated blocks as this case:

```cpp
    uint32* x = (uint32*)alloc(12);

    uint32* z = (uint32*)alloc(12);

    destroy(x);
    x = NULL; // to prevent use of freed address.

    uint32* y = (uint32*)alloc(8); // should be able to allocate in place of x.

    uint32* w = (uint32*)alloc(8); // findBlock bug, w lies between y & z but it has not enough space causing overlap with z.
                                   // yh | y | y | wh | w zh | w z | z | z -> OVERLAP!

```

## Fixing the overlapping issue:

The solution is to calculate the remaining size after reallocating the block with size less than the original size, then use it to create a new header for the remaining free block and mark it as dellocated by (setting `isAllocated` to false & `word` to the remaining size), this way we can reuse the remaining free block for future allocations without causing overlap with the next allocated block.

A special case: in case the remaining size is 1, that's enough for the header only, so we mark it as allocated and then it's considered fragmented memory that cannot be used for future allocations, but it won't cause any issues as it is marked as allocated and won't be reused.

## Restructing the build system to CMake.

- Setup languages as follows

    ```cmake
    # Setup NASM target architecture.
    if(APPLE)
        set(CMAKE_ASM_NASM_OBJECT_FORMAT macho32)
    elseif(UNIX)
        set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)
    elseif(WIN32)
        set(CMAKE_ASM_NASM_OBJECT_FORMAT win32)
    endif()

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    # Enable assembly with NASM
    enable_language(ASM_NASM)
    project(allocator VERSION 0.0.1 LANGUAGES CXX ASM_NASM)
    ```

- Setup C++ sources compiliation

    ```cmake
    # Set CXX Compiler options
    set(CXXFLAGS -Wall -m32)

    # Build cpp objects
    add_library(allocator_cpp OBJECT src/alloc.cpp)
    target_compile_options(allocator_cpp PRIVATE ${CXXFLAGS})
    ```

- Setup ASM sources compiliation

    ```cmake
    # Setup assembly files with ASM_NASM
    set(NASM_SOURCES src/heap.asm)
    set_source_files_properties(${NASM_SOURCES} ASM_NASM)

    # Build asm objects
    add_library(allocator_asm OBJECT ${NASM_SOURCES})
    ```

- link generated objects into a static library that can be used with other architectures.
    ```cmake
    add_library(${PROJECT_NAME} STATIC
                $<TARGET_OBJECTS:allocator_cpp>
                $<TARGET_OBJECTS:allocator_asm>
    )
    ```

- Build with CMAKE
    - Generate make build system
        ```bash
           mkdir build
           cd build
           cmake .. -G "Unix Makefiles" 
        ```

    - Build
        ```bash
         make
        ```

- Use the generated static lib with main to build test program.

    ```bash
        # Build test program and link it with the generated liballocator.a
        g++ main.cpp -m32 -L./build -lallocator -o test

        # Run
        ./test
    ```

## Next steps:

- Add Google Test to define some tests for allocator module

- Build for both unix 32 & 64 bit arch.

- Make the total memory arena for allocator dynamic.
    ```cpp
    // Example
    // make system allocate a block of memory on heap.
    void* addr = VirtualAlloc(1024);

    // Pass memory address to our allocator.
    Allocator memAlloc{addr};

    // Use our allocator.
    int x = (int*)memAlloc.alloc(sizeof(int));

    memAlloc.destroy(x);
    ```