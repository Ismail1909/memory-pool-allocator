#ifndef ALLOC_H
#define ALLOC_H

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

// attribute defs
#define packed __attribute__((__packed__)) // prevent internal padding between struct members.
#define unused __attribute__((__unused__))

#define ErrNoMem 12
#define ErrNoVal 13

#define reterrPtr(x) { errno = x; return 0;}

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
typedef struct s_header header;

void* alloc(uint32 bytes);
void destroy(void* address);


#endif // ALLOC_H