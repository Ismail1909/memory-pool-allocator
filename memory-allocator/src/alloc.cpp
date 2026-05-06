#include "alloc.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

// attribute defs
#define packed __attribute__((__packed__)) // prevent internal padding between struct members.
#define unused __attribute__((__unused__))

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

extern heap* memspace;

inline constexpr uint32 MAXWORDS = (1024*1024*1024) / 4;

void* mkalloc(header* hdr, word words) {
    word* LastAddr = ((word*)memspace) + MAXWORDS;
    if(words > MAXWORDS - 1) {
        reterrPtr(ErrNoMem);
    }

    //Check if there is available size relative to the header position
    if(((word*)(hdr) + words) > (LastAddr - 1)) {
        reterrPtr(ErrNoMem);
    }

    // Check if header is available.
    if(hdr->m_word) {
        // check if the header is freed & if it's enough.
        if(hdr->m_isAllocated || (hdr->m_word < words)) {
            reterrPtr(ErrNoMem);
        }
    }

    hdr->m_word = words;
    hdr->m_isAllocated = true;
    return (word*)hdr + 1; // return memory block address.
}

header* findBlock(header* hdr, word words) {
    word* LastAddr = ((word*)memspace) + MAXWORDS;
    header* retHdr = hdr;

    while((word*)retHdr < LastAddr - 1) {
        if(!retHdr->m_isAllocated) { // in use or fragmented.
            if(!retHdr->m_word) break; // unallocated block
            
            if(words <= retHdr->m_word) {
                if(words == retHdr->m_word) break; // no fragments.

                // Mark the rest of the free fragments as deallocated so it can detected later.
                word* freeHdrAddr = (word*)retHdr + words + 1;
                header* freeHdr = (header*)freeHdrAddr;
                freeHdr->m_word = retHdr->m_word - words - 1;
                // If n of words is zero, that means the free space is only enough for header & so it's fragmented and can't be used.
                // So we set it to true to prevent from being selected for allocation. ( TODO: do better logic??).
                freeHdr->m_isAllocated = (!freeHdr->m_word) ? true : false;        
                
                break;        
            }    
        }

        // Go to next possible header.
        word* next = (word*)retHdr + retHdr->m_word + 1;
        retHdr = (header*)next;
    }

    if((word*)retHdr > LastAddr - 1) {
        reterrPtr(ErrNoMem);
    }

    return retHdr;
}

void* alloc(uint32 bytes) {
    word words = 0;
    void* mem = memspace;
    header* hdr = 0;
    
    if(!bytes) reterrPtr(ErrNoVal);

    // Convert bytes to words
    words = (bytes%4) ?  ((bytes/4) + 1) : (bytes/4);

    // Check if no of words > max allowed
    if(words > MAXWORDS - 1)
    {
        reterrPtr(ErrNoMem);
    }

    // set header to first location
    hdr = (header*)mem;
    if(hdr->m_word) {
        // Find available block.
        hdr = findBlock(hdr, words);
        if(!hdr) {
            reterrPtr(errno);
        }
    }

    // make allocation in words
    void* allocated = mkalloc(hdr, words);
    if(!allocated) {
        reterrPtr(errno); // propagate error
    }

    return allocated;
}

void zero(word* addr, word words) {
    for(word i = 0 ; i < words ; ++i) {
        addr[i] = 0;
    }
}

void destroy(void* address) {
    if(!address) {
        return; // If NULL, do nothing.
    }

    // Get header of that memory block
    header* hdr = (header*)(((word*) address) - 1);

    if(!hdr->m_isAllocated) { // Not allocated, double free -> abort
        printf("Double free error");
        abort();
    }

    // Zero the used memory.
    zero((word*)address, hdr->m_word);

    // Mark header as deallocated
    hdr->m_isAllocated = false;

    return;
}

/*
int main() {
    printf("Mem start: %p \n", memspace);

    uint32* x = (uint32*)alloc(12);
    x[0] = 4;
    debug_print(x);

    uint32* z = (uint32*)alloc(12);
    *z = 5;
    debug_print(z);

    destroy(x);
    x = NULL; // to prevent use of freed address.
    //destroy(x); double free triggers abort as per standard.

    uint32* y = (uint32*)alloc(8); // should be able to allocate again.
    debug_print(y);

    uint32* w = (uint32*)alloc(8); // findBlock bug, w lies between y & z but it has not enough space causing overlap with z.
                                   // yh | y | y | wh | w zh | w z | z | z -> OVERLAP!
    debug_print(w);
    
    destroy(z);

    uint32* p = (uint32*)alloc(20);
    debug_print(p);

    uint32* j = (uint32*)alloc(12);
    debug_print(j);

    return 0;
}
*/