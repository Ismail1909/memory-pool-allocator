#include "src/alloc.h"

#include <stdio.h>

typedef unsigned int uint32;

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
    //printf("Mem start: %p \n", memspace);

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