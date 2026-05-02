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

int main() {
    char* x = (char*)alloc(12);
    if(x) {
        *x = 'O';
        *((uint8*)x + 10) = 4;
        printf("X: %c \n", *x);
        printf("X Addr: %p \n", x);
        printf("Mem start: %p \n", memspace);
    }
    else {
        printf("Failed to allocate mem\n");
    }

    destroy(x);
    x = NULL; // to prevent use of freed address.
    //destroy(x); double free triggers abort as per standard.

    char* y = (char*)alloc(8); // should be able to allocate again.
    if(!y) {
        printf("Failed to allocate Y \n");
    }
    else {
        *y = 'C';
        printf("Y: %c \n", *y);
        printf("Y Addr: %p \n", y);
        printf("Mem start: %p \n", memspace);
    }
    return 0;
}