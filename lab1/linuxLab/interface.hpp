#pragma once
#include <iostream>

class Interface
{
private:
    void runFileCreating(); //Создание файла
    void runFileCopying(); //КОпирование файла
    void runFileMoving(); //Перемещание файла
    void runDirectoryCreating(); //Создание директории
    void runDirectoryRemoving(); //Удаление директории
public:
    Interface();
    ~Interface();
    void show();
};
