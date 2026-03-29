#pragma once
#include <iostream>
#include "FileCopier.hpp"
#include "TestCopying.hpp"

class Interface
{
private:
    FileCopier fileCpy;
    TestCopying cpyTester;
    void runFileCopying();
    void runCopyTests();
    void runChangingAlign();
    void runChangingOverlapCount();
    void runFileDeleting();
public:
    Interface();
    ~Interface();
    void show();
};
