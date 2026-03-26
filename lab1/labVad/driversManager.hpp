#include <iostream>
#include <windows.h>
#include <vector>
#include <string.h>
#include <iomanip>
#include "tools.hpp"

using namespace std;

#define MAX_SIZE 250

void showVolumeInformation(const vector<string>& listofdrivers);
void showDriveType(const vector<string>& listofdrivers);
void showDrivesByGetLogicalDrives(vector<string>& listofdrivers);
void showDrivesByGetLogicalDrivestrings(vector<string>& listofdrivers);