#include "interface.hpp"

Interface::Interface(){
    system("clear");
    int Align, blockSizeMltpl;
    std::cout << "Введите выравнивание буфера в килобайтах: ";
    std::cin >> Align;
    std::cout << "Введите множитель для блока копирования (во сколько раз больше выравнивания): ";
    std::cin >> blockSizeMltpl;
    fileCpy.setAlign(Align, blockSizeMltpl);
}
Interface::~Interface(){}

void Interface::show(){
    bool running=true;
    int userChoice;
    bool wrongOption=false;
    while(running){
        system("clear");
        if(wrongOption){
            std::cout << "WRONG OPTION!\nTRY AGAIN!\n";
            wrongOption=false;
        }
        std::cout<<"=====Menu=====\n";
        std::cout<<"0.  Exit\n";
        std::cout<<"1.  Create file in directory"<<std::endl;
        std::cout<<"2.  Copy file in directory"<<std::endl;
        std::cout<<"Choose option: ";
        std::cin >> userChoice;
        system("clear");
        switch (userChoice)
        {
        case 0:
            running=false;
            std::cout << "=======EXITING=======\n";
            std::cout << "Exiting...\n";
            break;
        case 1:
            std::cout << "==========CREATE FILE==========\n";
            runFileCreating();
            break;
        case 2:
            std::cout << "==========COPY FILE==========\n";
            runFileCopying();
            break;
        default:
            wrongOption=true;
            break;
        }
        if(!wrongOption){
            std::cout << "Press ENTER to continue...\n";
            std::cin.ignore();
            std::cin.get();
        }
    }
}

void Interface::runCopyTests(){}

void Interface::runFileCopying(){
    std::string destDir, srcPath;
    std::cout << "Введите путь к файлу, который копировать: ";
    std::cin >> srcPath;
    std::cout << "Введите директорию, куда копировать файл: ";
    std::cin>>destDir;
    if(fileCpy.copyFile(srcPath, destDir)){
        std::cout << "Копирование успешно завершено\n";
    }
    else{
        std::cout << "Произошла ошибка!\n";
    }
}

void Interface::runFileCreating(){}