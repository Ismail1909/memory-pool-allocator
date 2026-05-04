#include "alloc.h"

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
        if(!retHdr->m_word) break; // unallocated block

        if(!retHdr->m_isAllocated) { // deallocated block
            if(words <= retHdr->m_word) {
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
    //zero((word*)address, hdr->m_word); // This function has logic error, it zeros beyond the allocated address

    // Mark header as deallocated
    hdr->m_isAllocated = false;

    return;
}

void debug_print(uint32* x) {
    if(!x) {
        printf("Failed to allocate Y \n");
    }
    else {
        printf("X: %d \n", *x);
        printf("X Addr: %p \n", x);
    }
}

int main() {
    printf("Mem start: %p \n", memspace);

    uint32* x = (uint32*)alloc(12);
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

    {
        uint32* x = (uint32*)alloc(12);
        debug_print(x);
    }

    {
        uint32* x = (uint32*)alloc(12);
        debug_print(x);
    }

    {
        uint32* x = (uint32*)alloc(12);
        debug_print(x);
    }

    {
        uint32* x = (uint32*)alloc(12);
        debug_print(x);
    }

    return 0;
}