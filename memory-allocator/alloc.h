#ifndef ALLOC_H
#define ALLOC_H

#include <stdio.h>

// attribute defs
#define packed __attribute__((__packed__)) // prevent internal padding between struct members.
#define unused __attribute__((__unused__))

// int typedefs
typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned int uint32;
typedef unsigned long int uint64;

// project types
typedef void heap;
typedef uint32 word;

struct packed s_header {
    word m_word:30; // number of words allocated
    bool m_isAllocated:1;
    bool unused m_reserved:1;
};


#endif // ALLOC_H