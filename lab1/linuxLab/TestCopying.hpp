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
    FileCopier fileCpy;
    void createTestFile(const std::string& filename);
    void createFilesForTest();
public:
    TestCopying();
    ~TestCopying();

    void doTests(const int _numTests, const FileCopier& _fileCpy);
    void deleteAllFilesInDir(const std::string& dir);
};