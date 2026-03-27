#include <iostream>
#include <windows.h>
#include <vector>
#include <string.h>
#include <iomanip>
#include "tools.hpp"
#include "fileManager.hpp"
#include "driversManager.hpp"

int main()
{   
    int flag = 1; 
    int menu;
    vector<string> listofdrivers;
    
    while(flag){
        cout<<"\n           Menu:\n";
        cout<<"(0)  Exit;"<<endl;
        cout<<"(1)  Output list of disks;"<<endl;
        cout<<"(2)  Output list of disks;"<<endl;
        cout<<"(3)  Driver Type;"<<endl;
        cout<<"(4)  Volume Information;"<<endl;
        cout<<"(5)  Disk free space;"<<endl;
        cout<<"(6)  Create directory;"<<endl;
        cout<<"(7)  Remove directory;"<<endl;
        cout<<"(8)  Create file in directory;"<<endl;
        cout<<"(9)  Copy file;"<<endl;
        cout<<"(10) Move file;"<<endl;
        cout<<"(11) Move file (extended);"<<endl;
        cout<<"(12) File attributes;"<<endl;
        cout<<"(13) Set attributes;"<<endl;
        cout<<"(14) Get file info;"<<endl;
        cout<<"(15) Get file time;"<<endl;
        cout<<"(16) Set file time;"<<endl;
        cout<<"\nChoose function: ";
        cin>>menu;
        cout<<"\n";
        
        switch(menu){
            case 1:  
                showLogicalDrives(listofdrivers);
                break;
            case 2:
                showLogicalDrivesSTR(listofdrivers);
                break;
            case 3:
                showDriveType(listofdrivers);
                break;
            case 4:
                showDriverInformation(listofdrivers);
                break;
            case 5:
                showDiskFreeSpace(listofdrivers);
                break;
            case 6:
            {
                string dirPath;
                string dirName;
                cin.ignore();
                cout<<"Enter directory Path: ";
                getline(cin, dirPath);
                cout<<"Enter directory name: ";
                getline(cin, dirName);
                createDirectory(dirPath, dirName);
                break;
            }
            case 7:
            {
                string dirPath;
                cin.ignore();
                cout<<"Enter directory Path: ";
                getline(cin, dirPath);
                removeDirectory(dirPath);
                break;
            }
            case 8:
            {
                string dirPath;
                string fileName;
                string content;
                cin.ignore();
                cout<<"Enter directory path: ";
                getline(cin, dirPath);
                cout<<"Enter file name: ";
                getline(cin, fileName);
                cout<<"Enter file content (Enter for empty): ";
                getline(cin, content);
                createFile(dirPath, fileName, content);
                break;
            }
            case 9:
            {
                string sourcePath;
                string destDir;
                cin.ignore();
                cout<<"Enter source file path: ";
                getline(cin, sourcePath);
                cout<<"Enter destination directory: ";
                getline(cin, destDir);
                copyFile(sourcePath, destDir);
                break;
            }
            case 10:
            {
                string sourcePath;
                string destPath;
                cin.ignore();
                cout<<"Enter source file path: ";
                getline(cin, sourcePath);
                cout<<"Enter destination: ";
                getline(cin, destPath);
                moveFile(sourcePath, destPath);
                break;
            }
            case 11: 
            {
                string sourcePath;
                string destPath;
                int flagChoice;
                cin.ignore();
                cout<<"Enter source file path: ";
                getline(cin, sourcePath);
                cout<<"Enter destination: ";
                getline(cin, destPath);
                
                cout<<"\nSelect MoveFileEx flags:"<<endl;
                cout<<"(1) Normal move;"<<endl;
                cout<<"(2) Replace existing file;"<<endl;
                cout<<"(3) Allow copy between different drives;"<<endl;
                cout<<"(4) Replace + Allow copy;"<<endl;
                cout<<"\nChoose flag option: ";
                cin>>flagChoice;
                
                DWORD flags = 0;
                switch(flagChoice){
                    case 1: 
                        flags = 0;
                        break;
                    case 2:
                        flags = MOVEFILE_REPLACE_EXISTING;
                        break;
                    case 3: 
                        flags = MOVEFILE_COPY_ALLOWED;
                        break;
                    case 4:
                        flags = MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED;
                        break;
                    default: 
                        flags = 0;
                        break;
                }
                
                moveFileEx(sourcePath, destPath, flags);
                break;
            }
            case 12:
            {
                string filePath;
                cin.ignore();
                cout<<"Enter file path: ";
                getline(cin, filePath);
                ShowFileAttributes(filePath);
                break;
            }
            case 13:
            {
                string filePath;
                cin.ignore();
                cout<<"Enter file path: ";
                getline(cin, filePath);
                SetFileAttributesMenu(filePath);
                break;
            }
            case 14:
            {
                string filePath;
                cin.ignore();
                cout<<"Enter file path: ";
                getline(cin, filePath);
                GetFileInformation(filePath);
                break;
            }
            case 15:
            {
                string filePath;
                cin.ignore();
                cout<<"Enter file path: ";
                getline(cin, filePath);
                GetFileTimes(filePath);
                break;
            }
            case 16:
            {
                string filePath;
                cin.ignore();
                cout<<"Enter file path: ";
                getline(cin, filePath);
                SetFileTimes(filePath);
                break;
            }
            case 0:
                flag = 0;
                break;
            default:
                cout<<"Invalid choice! Please select 0-16."<<endl;
                break;
        }
    }
    return 0;
}