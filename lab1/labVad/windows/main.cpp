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
                showDriverFreeSpace(listofdrivers);
                break;
            case 6:
            {
                string dirPath;
                string dirName;
                bool isDir;
                cin.ignore();
                cout<<"Enter directory Path: ";
                getline(cin, dirPath);
                //Проверка на существование родительской директории
                if(!checkPath(trimmed(dirPath), isDir) || !isDir){
                    cout<<"Error: Parent directory does not exist: "<<dirPath<<endl;
                    break;
                }
                cout<<"Enter directory name: ";
                getline(cin, dirName);
                string fullPath = trimmed(dirPath) + "\\" + trimmed(dirName);
                //Защита от повторение директорий
                if(checkPath(fullPath, isDir)){
                    if(isDir){
                        cout<<"Warning: Directory already exists!"<<endl;
                        break;
                    }
                }
                //Вызов API с нулевыми критериями безопастности 
                if(CreateDirectory(fullPath.c_str(), NULL)){
                    cout<<"Directory successfully created: "<<fullPath<<endl;
                    break;
                }else{
                    //Обработка исключений
                    DWORD error = GetLastError();
                    cout<<"Error: failed to create directory! Error code: "<<error<<endl;
                    break;
                }
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
                bool isDir;
                //Проверка существования родительской директории
                if(!checkPath(trimmed(dirPath), isDir)|| !isDir){
                    cout<<"Error: directory does not exist: "<<trimmed(dirPath)<<endl;
                    break;
                }
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
                bool isDir;
                if(!checkPath(trimmed(sourcePath), isDir)){
                    cout<<"Error: directory "<<trimmed(sourcePath)<<" is incorrect"<< endl;
                    break;
                }
                cout<<"Enter destination directory: ";
                getline(cin, destDir);
                if(!checkPath(trimmed(destDir), isDir)){
                    cout<<"Error: directory "<<trimmed(destDir)<<" is incorrect"<< endl;
                    break;
                }
                copyFile(sourcePath, destDir);
                break;
            }
            case 10:
            {
                string sourcePath;
                string destPath;
                bool isDir;
                cin.ignore();
                cout<<"Enter source file path: ";
                getline(cin, sourcePath);
                if(!checkPath(trimmed(sourcePath), isDir)){
                    cout<<"Error: source file does not exist!"<<endl;
                    break;
                }
                if(isDir){
                    cout<<"Error: source is a directory!"<<endl;
                    break;
                }   
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

                cin.ignore(); // очистка буфера

                cout << "Enter source file path: ";
                getline(cin, sourcePath);

                bool isDir;

                if (!checkPath(trimmed(sourcePath), isDir)) {
                    cout << "Error: source file does not exist!" << endl;
                    break;
                }

                if (isDir) {
                    cout << "Error: source is a directory!" << endl;
                    break;
                }

                cout << "Enter destination: ";
                getline(cin, destPath);

                bool destIsDir;
                string finalDestPath = destPath;
                
                if (checkPath(destPath, destIsDir) && destIsDir) {
                    size_t lastSlash = sourcePath.find_last_of("\\/");
                    string fileName = (lastSlash != string::npos) ? 
                                    sourcePath.substr(lastSlash + 1) : sourcePath;
                    if (destPath.back() != '\\' && destPath.back() != '/') {
                        finalDestPath += '\\';
                    }
                    finalDestPath += fileName;
                }

                cout << "\nSelect MoveFileEx flags:" << endl;
                cout << "(1) Normal move;" << endl;
                cout << "(2) Replace existing file;" << endl;
                cout << "(3) Allow copy between different drives;" << endl;
                cout << "\nChoose flag option: ";
                cin >> flagChoice;

                DWORD flags = 0;
                switch (flagChoice) {
                    case 1:flags = MOVEFILE_COPY_ALLOWED; break;
                    case 2:flags = MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED;break;
                    case 3:flags = MOVEFILE_COPY_ALLOWED; break;
                    default:
                        cout << "Invalid option. Using default (COPY_ALLOWED).\n";
                        flags = MOVEFILE_COPY_ALLOWED;
                        break;
                }

                moveFileEx(sourcePath, finalDestPath, flags);

                break;
            }
            case 12:
            {
                string filePath;
                cin.ignore();
                cout<<"Enter file path: ";
                getline(cin, filePath);
                showFileAttributes(filePath);
                break;
            }
            case 13:
            {
                string filePath;
                cin.ignore();
                cout<<"Enter file path: ";
                getline(cin, filePath);
                setFileAttribute(filePath);
                break;
            }
            case 14:
            {
                string filePath;
                cin.ignore();
                cout<<"Enter file path: ";
                getline(cin, filePath);
                showFileInfo(filePath);
                break;
            }
            case 15:
            {
                string filePath;
                cin.ignore();
                cout<<"Enter file path: ";
                getline(cin, filePath);
                showFileTimeInfo(filePath);
                break;
            }
            case 16:
            {
                string filePath;
                cin.ignore();
                cout<<"Enter file path: ";
                getline(cin, filePath);
                setFileTime(filePath);
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