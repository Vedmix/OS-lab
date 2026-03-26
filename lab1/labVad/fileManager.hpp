#include <iostream>
#include <windows.h>
#include <vector>
#include <string.h>
#include <iomanip>
#include "tools.hpp"

using namespace std;

bool createDirectory(const string& dirPath, const string& dirName);
bool removeDirectory(const string& dirPath);
bool createFile(const string& dirPath, const string& fileName, const string& content = "");
bool copyFile(const string& sourcePath, const string& destDir);
bool moveFile(const string& sourcePath, const string& destPath);
bool moveFileEx(const string& sourcePath, const string& destPath, DWORD flags = 0);
void showDiskFreeSpace(const vector<string>& listofdrivers);
void ShowFileAttributes(const string& filePath);
void SetFileAttributesMenu(const string& filePath);
void GetFileInformation(const string& filePath);
void SetFileTimes(const string& filePath);
void GetFileTimes(const string& filePath);