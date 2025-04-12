#pragma once

#include <iostream>
#include <malloc.h>
#include <unordered_set>
#include <memory>

struct malloc_struct {
	malloc_struct(void* paddr, size_t bytes, bool is_array, const char* file = " ", const char* func = " ", int line = -1) :
		m_paddr{ paddr }, m_bytes{ bytes }, m_is_array{ is_array }, m_file{ file }, m_func{ func }, m_line{ line } {
	}
	void* m_paddr;
	size_t m_bytes;
	const char* m_file;
	const char* m_func;
	int	  m_line;
	bool  m_is_array;
};

struct malloc_hash : std::hash<void *> {
	size_t operator()(const malloc_struct& p) const {
		return std::hash<void*>::operator()(p.m_paddr);
	}
};

template<typename T>
struct malloc_allocator_t : std::allocator<T> {
	T* allocate(std::size_t n) {
		T* p = (T*)std::malloc(n);
		if (!p) throw std::bad_alloc();

		return p;
	}

	void deallocate(T* p, std::size_t n) {
		std::free(p);
	}
};

std::unordered_set<malloc_struct, malloc_hash, std::equal_to<malloc_hash>,malloc_allocator_t<malloc_struct>> heap_entry_set;

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
