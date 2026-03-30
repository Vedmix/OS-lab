#include <iostream>
#include <windows.h>
#include <vector>
#include <string.h>
#include <iomanip>

using namespace std;

string trimmed(const string& str);
void printFileTimeForTime(const FILETIME& ftime,const string& labe);
void printFileTimeForHandle(const FILETIME& ft, const char* label);
bool checkPath(const string& path, bool& isDirectory);
string getFileName(const string& path);
string getCopyFileName(const string& originalPath);
bool checkContain(const string& fullPath);
