#include "interface.hpp"

Interface::Interface(){
    system("clear");
    runChangingAlign();
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
        std::cout<<"1.  Copy file in directory"<<std::endl;
        std::cout<<"2.  Do copy tests" << std::endl;
        std::cout<<"3.  Change buffer align" << std::endl;
        std::cout<<"4.  Delete all files in directories"<<std::endl;
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
            std::cout << "==========COPY FILE==========\n";
            runFileCopying();
            break;
        case 2:
            std::cout << "==========COPY TESTS==========\n";
            runCopyTests();
            break;
        case 3:
            std::cout << "==========CHANGE ALIGN==========\n";
            runChangingAlign();
            break;
        case 4:
            std::cout << "==========FILES DELETING==========\n";
            runFileDeleting();
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

void Interface::runCopyTests(){
    int numTests;
    std::cout << "Введите количество файлов для копирования: ";
    std::cin >> numTests;
    cpyTester.doTests(numTests, fileCpy);
    std::cout << "Тесты завершены\n";
}

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

void Interface::runChangingAlign(){
    int Align, blockSizeMltpl;
    std::cout << "Введите выравнивание буфера в килобайтах: ";
    std::cin >> Align;
    std::cout << "Введите множитель для блока копирования (во сколько раз больше выравнивания): ";
    std::cin >> blockSizeMltpl;
    fileCpy.setAlign(Align, blockSizeMltpl);
}

void Interface::runFileDeleting(){
    if(std::filesystem::exists("filesOrig")){
        if(std::filesystem::is_directory("filesOrig")){
            cpyTester.deleteAllFilesInDir("filesOrig");
        }
    }

    if(std::filesystem::exists("filesCopy")){
        if(std::filesystem::is_directory("filesCopy")){
            cpyTester.deleteAllFilesInDir("filesCopy");
        }
    }
    std::cout << "Files deleted\n";
}