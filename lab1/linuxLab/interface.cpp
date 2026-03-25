#include "interface.hpp"

Interface::Interface(){}
Interface::~Interface(){}

void Interface::show(){
    bool flag=true;
    int userChoice;
    bool wrongOption=false;
    while(flag){
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
        std::cout<<"5. Remove directory"<<std::endl;
        std::cout<<"Choose option: ";
        std::cin >> userChoice;
        system("clear");
        switch (userChoice)
        {
        case 0:
            flag=false;
            std::cout << "=======EXITING=======\n";
            std::cout << "Exiting...\n";
            break;
        case 1:
            std::cout << "==========CREATE FILE==========\n";

            break;
        case 2:
            std::cout << "==========COPY FILE==========\n";

            break;
        case 3:
            std::cout << "==========MOVE FILE==========\n";

            break;
        case 4:
            std::cout << "==========CREATE DIRECTORY==========\n";

            break;
        case 5:
            std::cout << "==========REMOVE DIRECTORY==========\n";

            break;
        default:
            wrongOption=true;
            break;
        }
        if(!wrongOption){
            std::cout << "Press ENTER to continue...\n";
            getchar();
            getchar();
        }
    }
}