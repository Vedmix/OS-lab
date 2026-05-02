#pragma once
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
    void runBlockSizeAutoTest(const int _numTests, const size_t _fileSize, const int numCycles);
    void runOLOAutoTest(const int _numTests, const size_t _fileSize, const int numCycles);
    void showParameteres();
    void showMenu();
public:
    Interface();
    ~Interface();
    void show();
};
