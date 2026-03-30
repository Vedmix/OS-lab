#include <iostream>
#include <windows.h>
#include <vector>
#include <string.h>
#include <iomanip>
#include "tools.hpp"

using namespace std;

void removeDirectory(const string& dirPath);

void createFile(const string& dirPath, const string& fileName, const string& content = "");
void copyFile(const string& sourcePath, const string& destDir);
void moveFile(const string& sourcePath, const string& destPath);
void moveFileEx(const string& srcPath, const string& dstPath, DWORD flags);

void showFileAttributes(const string& filePath);
void setFileAttribute(const string& filePath);
void showFileInfo(const string& filePath);
void setFileTime(const string& filePath);
void showFileTimeInfo(const string& filePath);