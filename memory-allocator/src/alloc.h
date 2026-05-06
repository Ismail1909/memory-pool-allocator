#ifndef ALLOC_H
#define ALLOC_H

// Error Macros
#define ErrNoMem 12
#define ErrNoVal 13

void* alloc(unsigned int bytes);
void destroy(void* address);


#endif // ALLOC_H