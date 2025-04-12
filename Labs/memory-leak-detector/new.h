#pragma once

#include <iostream>
#include <malloc.h>
#include <unordered_set>
#include <memory>


void* operator new(size_t size) {
	std::cout << "new operator called" << std::endl;
	if (size <= 0) {
		throw std::bad_alloc();
	}

	void* p = (void*) std::malloc(size);
	if (!p) {
		throw std::bad_alloc();
	}

	return p;
}

void* operator new(size_t size, const char* file, int line, const char* func) {
	std::cout << "new operator called at " << file << " Line: " << line << " at Function: " << func << std::endl;
	return ::operator new(size);
}

void* operator new [](size_t size) {
	std::cout << "new [] operator called" << std::endl;
	void* p = (void*)std::malloc(size);
	if (!p) {
		throw std::bad_alloc();
	}

	return p;
}

void operator delete(void* p) {
	std::cout << "delete operator called" << std::endl;
	std::free(p);
}

void operator delete [](void* p) {
	std::cout << "delete [] operator called" << std::endl;
	std::free(p);
}

#define new new(__FILE__, __LINE__, __FUNCSIG__)
