#pragma once
#include <chrono>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include "FileCopier.hpp"

class TestCopying
{
private:
    int numTests;
    size_t fileSize;
    FileCopier fileCpy;
    void createTestFile(const std::string& filename);
    void createFilesForTest();
    void doCopying();
public:
    TestCopying();
    ~TestCopying();

    void doTests(const int _numTests, const FileCopier& _fileCpy, const size_t _fileSize);
    void deleteAllFilesInDir(const std::string& dir);
};