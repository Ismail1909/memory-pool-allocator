#pragma once

#include <iostream>
#include <malloc.h>
#include <unordered_set>
#include <memory>
#include <ostream>

struct malloc_struct {
	malloc_struct(void* paddr = nullptr, size_t bytes = 0, bool is_array = false, const char* file = " ", const char* func = " ", int line = -1) :
		m_paddr{ paddr }, m_bytes{ bytes }, m_is_array{ is_array }, m_file{ file }, m_func{ func }, m_line{ line } {
	}
	void* m_paddr;
	size_t m_bytes;
	const char* m_file;
	const char* m_func;
	int	  m_line;
	bool  m_is_array;

	bool operator == (const malloc_struct& obj) const {
		return (this->m_paddr == obj.m_paddr);
	}

};

std::ostream& operator <<(std::ostream& os, const malloc_struct& entry) {
	os << "Heap Entry >> Address: " << entry.m_paddr << " | Bytes: " << entry.m_bytes << " At File: " << entry.m_file
		<< " | Line: " << entry.m_line << " | Function: " << entry.m_func;
	return os;
}

struct malloc_hash : std::hash<void *> {
	size_t operator()(const malloc_struct& p) const {
		return std::hash<void*>::operator()(p.m_paddr);
	}
};

template<typename T>
struct malloc_allocator_t : std::allocator<T> {

	malloc_allocator_t() = default;

	template<class U>
	malloc_allocator_t(const malloc_allocator_t<U>&) noexcept {}

	T* allocate(std::size_t n) {
		T* p = (T*)std::malloc(n * sizeof(T));
		if (!p) throw std::bad_alloc();

		return p;
	}

	void deallocate(T* p, std::size_t n) {
		std::free(p);
	}

	template<typename U>
	struct rebind { typedef malloc_allocator_t<U> other; };
};

std::unordered_set<malloc_struct, malloc_hash, std::equal_to<malloc_struct>,malloc_allocator_t<malloc_struct>> heap_entry_set;

std::vector<malloc_struct, malloc_allocator_t<malloc_struct>> memory_alloc_mismatch;

void* operator_new(size_t size, malloc_struct& entry) {
	std::cout << "new operator called" << std::endl;
	if (size <= 0) {
		throw std::bad_alloc();
	}

	void* p = (void*) std::malloc(size);
	if (!p) {
		throw std::bad_alloc();
	}
	entry.m_paddr = p;
	heap_entry_set.insert(std::forward<decltype(entry)>(entry));
	return p;
}

void* operator new(size_t size, const char* file, int line, const char* func) {
	std::cout << "new operator called at " << file << " Line: " << line << " at Function: " << func << std::endl;
	malloc_struct entry{ nullptr,size,false,file,func,line };
	return operator_new(size,entry);
}

void* operator new[](size_t size, const char* file, int line, const char* func) {
	std::cout << "new [] operator called at " << file << " Line: " << line << " at Function: " << func << std::endl;
	malloc_struct entry{ nullptr,size,true,file,func,line };
	return operator_new(size, entry);
}

void operator_delete(void* p, bool is_array) {
	auto it = heap_entry_set.find(p);
	if (it != heap_entry_set.end()) {
		if (it->m_is_array != is_array) {
			memory_alloc_mismatch.push_back(*it);
		}
		heap_entry_set.erase(it);
	}
	std::free(p);
}

void operator delete(void* p) {
	std::cout << "delete operator called" << std::endl;
	operator_delete(p, false);
}

void operator delete [](void* p) {
	std::cout << "delete [] operator called" << std::endl;
	operator_delete(p, true);
}

void trace_memory_leaks() {
	std::cout << "Memory leaks detected for: " << std::endl;
	for (auto &entry : heap_entry_set) {
		std::cout << entry << std::endl;
	}
}

void trace_memory_delete_mismatch() {
	std::cout << "Memory delete mismatch detected for: " << std::endl;
	for (auto& entry : memory_alloc_mismatch) {
		std::cout << entry << std::endl;
	}
}


struct memory_leak_trace {
	memory_leak_trace() = default;
	~memory_leak_trace() {
		trace_memory_leaks();
		trace_memory_delete_mismatch();
	}
};

#define new new(__FILE__, __LINE__, __FUNCSIG__)
