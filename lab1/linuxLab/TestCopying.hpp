#pragma once
#include <chrono>

class TestCopying
{
public:
    TestCopying();
    ~TestCopying();

    std::chrono::microseconds doTests(const int numTests);
};