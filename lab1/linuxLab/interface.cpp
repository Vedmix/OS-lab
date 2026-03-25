#include "interface.hpp"

Interface::Interface(){}
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
        std::cout<<"3.  Move file in directory"<<std::endl;
        std::cout<<"4.  Create directory"<<std::endl;
        std::cout<<"5.  Remove directory"<<std::endl;
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
        case 3:
            std::cout << "==========MOVE FILE==========\n";
            runFileMoving();
            break;
        case 4:
            std::cout << "==========CREATE DIRECTORY==========\n";
            runDirectoryCreating();
            break;
        case 5:
            std::cout << "==========REMOVE DIRECTORY==========\n";
            runDirectoryRemoving();
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