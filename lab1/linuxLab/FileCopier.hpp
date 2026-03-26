#pragma once
#include <string>
#include <libaio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <filesystem>

class FileCopier
{
private:
    size_t ALIGN; //выравнивание буфера в байтах
    size_t blockSize; //размер блока копирования (кратен ALIGN)
public:
    FileCopier();
    FileCopier(const size_t _ALIGN, const int blockSizeMultiplier);
    ~FileCopier();
    bool copyFile(const std::string& srcPath, const std::string& destDir);

    void setAlign(const size_t _ALIGN, const int blockSizeMultiplier);
};