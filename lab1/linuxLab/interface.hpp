#pragma once
#include <iostream>
#include "FileCopier.hpp"
#include "TestCopying.hpp"

class Interface
{
private:
    FileCopier fileCpy;
    TestCopying cpyTester;
    void runFileCopying(); //КОпирование файла
    void runCopyTests();
    void runChangingAlign();
public:
    Interface();
    ~Interface();
    void show();
};
