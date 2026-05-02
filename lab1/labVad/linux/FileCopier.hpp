#pragma once
#include <string>
#include <libaio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <filesystem>
#include <vector>

class FileCopier
{
private:
    size_t ALIGN; //выравнивание буфера в байтах
    size_t blockSize; //размер блока копирования (кратен ALIGN)
    int overlapCount; //число перекрывающих операций ввода и вывода
public:
    FileCopier();
    FileCopier(const size_t _ALIGN, const int blockSizeMultiplier);
    ~FileCopier();
    bool copyFile(const std::string& srcPath, const std::string& destDir);

    FileCopier& operator=(const FileCopier& _fileCpy);
    void setAlign(const size_t _ALIGN, const int blockSizeMultiplier); //ALIGN в килобайтах
    void setOverlapCount(const int _overlapCount);

    size_t getAlign();
    size_t getBlockSize();
    int getOverlapCount();
};