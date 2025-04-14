#pragma once

class Integer {

public:
    Integer() {
        m_num = new int{ 0 };
    }
    Integer(int num) {
        m_num = new int{ num };
    }
    ~Integer() {

    }

private:
    int* m_num = nullptr;
};

