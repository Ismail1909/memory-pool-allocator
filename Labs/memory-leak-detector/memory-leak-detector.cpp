// memory-leak-detector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "new.h"
#include "Integer.h"

struct S1 {
    int x;
    int y;
    int z;
    S1() = default;
    S1(int x, int y, int z) : x(x), y(y), z(z) {}
};

int main()
{
    memory_leak_trace trace;
    try {
        char* p1 = new char[1];
        long* l = new long;
        std::shared_ptr<int> p2{ new int };
        S1* s1 = new S1;
        Integer i{ 5 };
        *p1 = '5';
        *p2.get() = 15;
        *l = 100;
        std::cout << *p1 << std::endl;
        //delete[] p1;
        delete p1;
        delete s1;
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}