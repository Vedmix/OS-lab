#pragma once
#include <iostream>
#include "FileCopier.hpp"
#include "TestCopying.hpp"

class Interface
{
private:
    FileCopier fileCpy;
    TestCopying cpyTests;
    void runFileCreating(); //Создание файла
    void runFileCopying(); //КОпирование файла
    void runCopyTests();
public:
    Interface();
    ~Interface();
    void show();
};
