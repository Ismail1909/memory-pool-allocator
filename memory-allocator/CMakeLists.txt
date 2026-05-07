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

# Set CXX Compiler options, it is better to use the CMAKE flag in this case to unify the arch with
# external deps that are built like gtest.
set(CMAKE_CXX_FLAGS -m32)

# Enable assembly with NASM
enable_language(ASM_NASM)
project(allocator VERSION 0.0.1 LANGUAGES CXX ASM_NASM)

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare( # The hash represents a commit, update it regularly
  googletest
  URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
)
FetchContent_MakeAvailable(googletest)

# Build cpp objects
add_library(allocator_cpp OBJECT src/alloc.cpp)

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

# Enable Google Test
enable_testing()

# Build test binary & link it to gtest & the allocator library
add_executable(
  allocator_tests
  tests/allocator_tests.cpp
)
target_include_directories(allocator_tests PUBLIC ${CMAKE_SOURCE_DIR}/src) # to include alloc header

target_link_libraries(
  allocator_tests
  GTest::gtest_main
  allocator
)

# Add the test binary to gtest.
include(GoogleTest)
gtest_discover_tests(allocator_tests)



