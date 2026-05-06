cmake_minimum_required(VERSION 3.14)

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

# Set CXX Compiler options
set(CXXFLAGS -Wall -m32)

# Build cpp objects
add_library(allocator_cpp OBJECT src/alloc.cpp)
target_compile_options(allocator_cpp PRIVATE ${CXXFLAGS})

# Setup assembly files with ASM_NASM
set(NASM_SOURCES src/heap.asm)
set_source_files_properties(${NASM_SOURCES} ASM_NASM)

# Build asm objects
add_library(allocator_asm OBJECT ${NASM_SOURCES})

# Build final static allocator library
add_library(${PROJECT_NAME} STATIC
            $<TARGET_OBJECTS:allocator_cpp>
            $<TARGET_OBJECTS:allocator_asm>
)



