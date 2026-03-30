#include <iostream>
#include <windows.h>
#include <vector>
#include <string.h>
#include <iomanip>
#include "tools.hpp"

using namespace std;

#define MAX_SIZE 250

void showLogicalDrives(vector<string>& listofdrivers); //Вывод списка дисков (через битовую маску)
void showLogicalDrivesSTR(vector<string>& listofdrivers); //Вывод списка дисков (через список строк)
void showDriveType(const vector<string>& listofdrivers); //Вывод типа диска
void showDriverInformation(const vector<string>& listofdrivers);//Вывод детальной  информации о томе
void showDriverFreeSpace(const vector<string>& listofdrivers); //Получает информацию о свободном месте на диске