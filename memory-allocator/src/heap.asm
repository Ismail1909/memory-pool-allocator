bits 32
global memspace

%define HeapSize (1024*1024*1024/4)

section .heap noexec nobits write align=4
    __memspace:
        heapSize equ HeapSize
        resd heapSize

section .data
    memspace:
        dd __memspace
