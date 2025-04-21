// boost-memory-pool.cpp
// A Sample program to try out boost memory pool allocator.
//

#include <vector>
#include <cstddef>
#include <iostream>

#include <boost/pool/simple_segregated_storage.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/pool/singleton_pool.hpp>

// Timer includes
#include <chrono>
#include <thread>

using namespace std::chrono;

struct timer
{
    void start() {
        m_t = high_resolution_clock::now();
    }

    void stop() {
        auto e = high_resolution_clock::now() - m_t;
        std::cout << "Time" << duration_cast<microseconds>(e).count() << " us" << std::endl;
    }

    high_resolution_clock::time_point m_t = high_resolution_clock::now();
};

// A tag to highlight that singleton_int_pool is a pool that manages int values.
struct int_pool {};

//<typename Tag, unsigned RequestedSize, typename UserAllocator, typename Mutex, unsigned NextSize, unsigned MaxSize >
// Tag: represents a unique type identifier for this pool.Thanks to tags, multiple singletons can manage different memory pools,
//      even if the second template parameter for the size is the same. 
// RequestedSize: the size of each allocated memory chunck/block.
// UserAllocator: user defined memory allocator, default = <tt>default_user_allocator_new_delete</tt>.
// Mutex: The type of mutex used to synchonise access to this pool.
// NextSize: the number of chunks allocated on first allocation.
// MaxSize: The max next number of chunks to allocate in case no free chunks in the allocated memory. 
typedef boost::singleton_pool<int_pool, sizeof(int)> singleton_pool_int;

int main()
{

    goto SINGELTON_POOL;

    // Dynamic memory allocation on heap has some problems and limits:
    // 1- Memory allocated on heap often requires some header data to store info for the memory allocated
    //    such as its position in a list structure, size, etc. This cause memory overhead especially for
    //    small chunks of memory that could exponentially waste a great chunk of memory in vain.
    // 2- System memory allocations are usually slow so that could affect time-critical operations that use the heap.

    // Solution: Boost pool library offers an implementation of simple segregated storage algorithm.
    // The basic idea is that it allocates a decent size of memory on heap and then segment it to blocks of same size and
    // it just keeps a list of free blocks and doesn't store any overhead on memory segments.
    // Now when the program requests memory from this storage, it returns a pointer to the first free block if exists.
    // 
    // Therefore it offers:
    // 1- Almost no memory overhead.
    // 2- All allocations take place in a small almost constant time, so faster allocations.
    // But this comes with a loss of generality as it can allocate memory chunks of a single size.


    /**************** Using boost::simple_segregated_storage ******************/

    // Boost.Pool provides the class boost::simple_segregated_storage to create and manage segregated memory.
    // It is a low-level class, so it's mainly used in embedded applications as it gives you control and responsibility of
    // how you allocate the memory, segment ,etc.
    {
        boost::simple_segregated_storage<size_t> storage;

        // We need to allocate 1024 bytes of contigous memory on heap to provide to the storage instance. We do so using std::vector.
        std::vector<char> mem(1024, 0);

        // The address of the first byte of this memory.
        char* addr = &mem.front();

        // Here we assign the control of this memory to the simple_segregated_storage instance and
        // We instruct it to segment it to 4 equal blocks of 256 bytes.
        storage.add_block(addr, mem.size(), 256);

        // malloc() returns pointer to a free block if exists.
        // Here we assign a pointer to the first segment to i.
        int* i = reinterpret_cast<int*>(storage.malloc());
        if (i) {
            *i = 4;
        }

        // malloc_n(n,size) returns a pointer to 'n' contiguous segments that provide 'size' bytes in one block as requested.
        // Here we assign a pointer to the 2nd and 3rd segments of the memory to j.
        unsigned int* j = reinterpret_cast<unsigned int*>(storage.malloc_n(2, 256));
        if (j) {
            *j = -1; // 2^32 -1 in memory
        }

        // k takes the last segment.
        int* k = reinterpret_cast<int*>(storage.malloc());
        if (k) {
            *k = INT_MAX; // 2^31 - 1
        }

        // This call will cause exception as there are no free block in this memory segment.
        //int* y = reinterpret_cast<int*>(storage.malloc());

        if (storage.empty()) // Check if there are free blocks
        {
            std::cout << "No free segments left" << std::endl;
        }

        storage.free(i);
        storage.free_n(j, 2, 256);
        storage.free(k);
    }
    goto END;

    /**************************************************************************/

    /************************ Using boost::object_pool ***********************/

OBJECT_POOL:

    // boost::object_pool is a high level interface to simple_segregated_storage with predetermint object type that 
    // will be stored in memory. The memory managed by pool consists of segments, each of which is the size of an int – 4 bytes for example.
    // You don't need to allocate memory for it as it allocates memory directly.
    {
        boost::object_pool<int> pool;

        // malloc() returns the first free segment of determined type. When called for the first time,
        // it allocates (32 segments by default) from memory if available.
        int* i = pool.malloc();
        if (i) {
            *i = INT_MAX;
        }
        // In this example further calls to malloc will return a pointer to the first free segment directly so a much
        // faster performance.

        // Allocating remaining 31 segments previously allocated.
        int* arr[31]{ nullptr };

        timer t;
        t.start();
        for (int j = 0; j < 30; ++j) {
            arr[j] = pool.malloc();
        }
        t.stop(); // 1 micro sec in debug

        // Comparing time vs standard dynamic allocation
        t.start();
        int* darr = new int[31];
        t.stop(); // 3 micro sec in debug
        delete[] darr;

        // destroy(Type*) calls the destructor of the type and deallocates the memory while
        // free(Type*) only deallocates the memory.
        pool.destroy(i);
        for (int j = 0; j < 30; ++j) {
            pool.destroy(arr[j]);
        }

        // construct(TYPE PARAMS..) same as malloc() but you can pass the parameters of type constructor.
        int* k = pool.construct(INT_MAX);
        std::cout << "K: " << *k << std::endl;
        pool.destroy(k);

        // This is the parameterized ctor, the first param denotes size of the memory block
        // that boost::object_pool will allocate when the first segment is requested( default is 32) while
        // the second param denotes the maximum size the pool can allocate incase there is no free blocks,
        // the default is 0 which indicates no limits. In object_pool the size of next allocation grows 
        // exponentially meaning that if the first 32 blocks are allocated, then it will allocate 64 blocks and so on.
        boost::object_pool<int> pool2{ 32,0 }; // default params

        for (int j = 0; j < 30; ++j) {
            arr[j] = pool2.malloc();
        }

        // gets the next segment of memory that will be allocated.
        std::cout << "Next size: " << pool2.get_next_size(); // 64

        // changes the size of the next memory block to allocate.
        pool2.set_next_size(8);

        for (int j = 0; j < 30; ++j) {
            pool2.destroy(arr[j]);
        }
    }

    goto END;

    /**************************************************************************/

    /********************* Using boost::singleton_pool ***********************/

SINGELTON_POOL:
    // Singleton Usage is the method where each Pool is an object with static duration; that is, it will not be destroyed until program exit.
    // Pool objects with Singleton Usage may be shared; thus, Singleton Usage implies thread-safety as well.
    
    {
        // Similar in behavior to object_pool, but like simple_segregation_storage it doesn't manage the type
        // but only the size of the chunk so it returns 'void*' that should be cast.
        int* i = reinterpret_cast<int*>(singleton_pool_int::malloc());
        *i = 2;

        // same as malloc() but allocates enough contigous chunks to cover n* sizeof(chunk) bytes of memory.
        int* j = static_cast<int*>(singleton_pool_int::ordered_malloc(10));
        j[9] = 2;

        // releases all memory blocks that aren’t used at the moment.
        singleton_pool_int::release_memory();

        // releases all memory blocks – including those currently being used.
        singleton_pool_int::purge_memory();
    }

END: 
    return 0;
}