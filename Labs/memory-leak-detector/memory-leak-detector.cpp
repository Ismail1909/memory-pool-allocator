// memory-leak-detector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "new.h"

int main()
{
    try {
        char* p1 = new char[0];
        *p1 = '5';
        std::cout << *p1 << std::endl;
        delete[] p1;
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}