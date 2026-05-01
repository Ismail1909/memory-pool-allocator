#include "alloc.h"

extern heap* memspace;

int main() {
    int* x = 0;
    x = (int*)memspace;
    *x = 4;
    return 0;
}