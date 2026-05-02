#include "interface.hpp"

Interface::Interface(){
    system("clear");
    runChangingAlign();
    runChangingOverlapCount();
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
        showParameteres();
        showMenu();
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
            showParameteres();
            std::cout << "==========COPY FILE==========\n";
            runFileCopying();
            break;
        case 2:
            showParameteres();
            std::cout << "==========COPY TESTS==========\n";
            runCopyTests();
            break;
        case 3:
            showParameteres();
            std::cout << "==========CHANGE ALIGN==========\n";
            runChangingAlign();
            break;
        case 4:
            showParameteres();
            std::cout << "==========CHANGE OVERLAPPING OPS==========\n";
            runChangingOverlapCount();
            break;
        case 5:
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
    int numTests, numCycles;
    size_t fileSize;
    char userChoice;
    std::cout << "Введите количество файлов для копирования: ";
    std::cin >> numTests;
    std::cout << "Введите размер создаваемых файлов (в килобайтах): ";
    std::cin >> fileSize;
    std::cout << "Введите количество прогонов: ";
    std::cin >> numCycles;
    std::cout << "Прогнать автотест? (y/n): ";
    std::cin >> userChoice;
    if(userChoice=='y'){
        std::cout << "Прогнать автотест размера блока, или количества перекрывающих операций (введите 1 или 2): ";
        std::cin >> userChoice;
        if(userChoice=='1'){
            runBlockSizeAutoTest(numTests, fileSize, numCycles);
        }
        else if(userChoice=='2'){
            runOLOAutoTest(numTests, fileSize, numCycles);
        }
    }
    else{
        cpyTester.doTests(numTests, fileCpy, fileSize, numCycles);
    }
    std::cout << "Тесты завершены\n";
}

void Interface::runBlockSizeAutoTest(const int _numTests, const size_t _fileSize, const int numCycles){
    int maxMultipl;
    std::cout << "Введите максимальный блок копирования (в кб) (шаг +2): ";
    std::cin >> maxMultipl;
    for(int i=1;i<=maxMultipl;i+=2){
        fileCpy.setAlign(1, i);
        std::cout << "Размер блока: " << i << " кб\n";
        if(i==1){
            i--;
        }
        cpyTester.doTests(_numTests, fileCpy, _fileSize, numCycles);
        std::cout << std::endl;
    }
}

void Interface::runOLOAutoTest(const int _numTests, const size_t _fileSize, const int numCycles){
    int maxOLO;
    std::cout << "Введите максимальное количество перекрывающих операций (шаг +2): ";
    std::cin >> maxOLO;
    for(int i=1;i<=maxOLO;i+=2){
        fileCpy.setOverlapCount(i);
        std::cout << "Количество перекрывающих операций: " << i << '\n';
        if(i==1){
            i--;
        }
        cpyTester.doTests(_numTests, fileCpy, _fileSize, numCycles);
        std::cout << std::endl;
    }
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

void Interface::runChangingOverlapCount(){
    int overlapCount;
    std::cout << "Введите число перекрывающих операций ввода и вывода: ";
    std::cin >> overlapCount;
    fileCpy.setOverlapCount(overlapCount);
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

void Interface::showParameteres(){
    std::cout<<"=====Parameters=====\n";
    std::cout<<"ALIGN: "<<fileCpy.getAlign()<<std::endl;
    std::cout<<"Block size: "<<fileCpy.getBlockSize()<<std::endl;
    std::cout<<"Number of overlapping operations: "<<fileCpy.getOverlapCount()<<std::endl;
}

void Interface::showMenu(){
    std::cout<<"=====Menu=====\n";
    std::cout<<"0.  Exit\n";
    std::cout<<"1.  Copy file in directory"<<std::endl;
    std::cout<<"2.  Do copy tests" << std::endl;
    std::cout<<"3.  Change buffer align" << std::endl;
    std::cout<<"4.  Change number of overlapping operations" << std::endl;
    std::cout<<"5.  Delete all files in directories"<<std::endl;
}