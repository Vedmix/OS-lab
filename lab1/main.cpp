#include <iostream>
#include <windows.h>
#include <vector>
#include <string.h>
#define MAX_SIZE 100




int main()
{   

    char lpBuffer[MAX_SIZE];
    int flag = 1; 
    char d;
    int menu, type, count, id;
    std::vector<std::string> listofdrivers;
    while(flag){
        std::cout<<"\n=====Menu=====\n";
        std::cout<<"1. Output list of disks (GetLogicalDriveStrings)\n";
        std::cout<<"2. Output list of disks (GetLogicalDrives)\n";
        std::cout<<"3. Driver Type (GetDriverType)\n";
        std::cout<<"0. Exit\n\n";
        std::cin>>menu;
        std::cout<<"\n";
        count = 1;
        switch(menu){
            case 1:  
            {
                count = 1;
                listofdrivers.clear();
                DWORD result =  GetLogicalDriveStringsA(MAX_SIZE, lpBuffer); //string buffer (ANSI)
                if((result > 0) && (result<=MAX_SIZE)){
                    std::cout<<"Your drives: "<<std::endl;
                    char* drive = lpBuffer;
                    while(*drive){
                        std::cout<<count<<". "<<drive<<std::endl;
                        listofdrivers.push_back(std::string(drive));
                        drive += strlen(drive) + 1;
                        count++;
                    }
                }
            }
            break;
            case 2:
            {   
                listofdrivers.clear();
                count = 1;
                DWORD drives = GetLogicalDrives(); //bit mask
                std::cout<<"Your drives: "<<std::endl;
                for(d = 'A';d <= 'Z'; d++){
                    if(drives & 1){
                        std::string disk = std::string(1,d) + ":\\";
                        std::cout<<count<<". "<<disk<<std::endl;
                        listofdrivers.push_back(disk);
                        count++;
                    }
                    drives >>= 1;
                }
            }
            break;
            case 3:
            {
                if(listofdrivers.empty()){
                    std::cout<<"Please check your drives first (Option 1-2 in menu)"<<std::endl; 
                    break;
                }
                std::cout<<"Please select a disk number"<<std::endl;
                std::cin>>id;
                UINT type = GetDriveTypeA(listofdrivers[id].c_str());
                switch(type){
                    case 0:{
                    std::cout<<"DRIVE_UNKNOWN (The drive type cannot be determined. )"<<std::endl;
                    }
                    break;
                    case 1:{
                    std::cout<<"DRIVE_NO_ROOT_DIR (The root path is invalid; for example, there is no volume mounted at the specified path. )"<<std::endl;
                    }
                    break;
                    case 2:{
                    std::cout<<"DRIVE_REMOVABLE (The drive has removable media; for example, a floppy drive, thumb drive, or flash card reader. )"<<std::endl;
                    }
                    break;
                    case 3:{
                    std::cout<<"DRIVE_FIXED (The drive has fixed media; for example, a hard disk drive or flash drive.)"<<std::endl;
                    }
                    break;
                    case 4:{
                    std::cout<<"DRIVE_CDROM (The drive is a remote (network) drive. )"<<std::endl;
                    }
                    break;
                    case 5:{
                    std::cout<<"DRIVE_CDROM (The drive is a CD-ROM drive. )"<<std::endl;
                    }
                    break;
                    case 6:{
                    std::cout<<"DRIVE_RAMDISK (The drive is a RAM disk. )"<<std::endl;
                    }
                    break;
                }
            }
            break;
            case 0:
            flag = 0;
            break;
        }
}
    return 0;
}

    