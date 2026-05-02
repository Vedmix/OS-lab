#include <iostream>
#include <windows.h>
#include <vector>
#include <string.h>
#include <iomanip>
#define MAX_SIZE 512



std::string trimmed(const std::string& str){
    size_t first = str.find_first_not_of(" \t\n\r");
    if(first == std::string::npos)return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

void printFileTimeT(const FILETIME& ftime,const std::string& labe){
        if(ftime.dwHighDateTime == 0 && ftime.dwLowDateTime ==0){
            std::cout<<labe<<": Not avalible"<<std::endl;
            return;
        }
        SYSTEMTIME stime;
        FileTimeToSystemTime(&ftime,&stime);

        std::cout<<labe<<": "<<std::setfill('0')<<std::setw(2)<<stime.wDay<<"."<<std::setw(2)<<stime.wMonth<<"."<<std::setw(4)<<stime.wYear<<" "<<std::setw(2)<<stime.wHour<<":"<<std::setw(2)<<stime.wMinute<<":"<<std::setw(2)<<stime.wSecond<<std::setfill(' ')<<std::endl;
    }

void printFileTimeH(const FILETIME& ft, const char* label){
	SYSTEMTIME st;
	FileTimeToSystemTime(&ft, &st);
	std::cout<<label<<": "<<std::setfill('0')<< st.wDay<<"."<<std::setw(2)<<st.wMonth<<"."<<st.wYear<<" "<< std::setw(2)<<st.wHour<<":"<<std::setw(2)<<st.wMinute<<":"<<std::setw(2)<<st.wSecond<<std::endl;
}


bool checkPath(const std::string& path, bool& isDirectory){
    DWORD attr = GetFileAttributes(path.c_str());
    if(attr == INVALID_FILE_ATTRIBUTES){
        return false;
    }
    
    isDirectory = (attr & FILE_ATTRIBUTE_DIRECTORY)!= 0;
    return true;
}

std::string getFileName(const std::string& path){
    size_t pos = path.find_last_of("\\");
    if(pos != std::string::npos){
        return path.substr(pos + 1);
    }
    return path;
}

std::string getCopyFileName(const std::string& originalPath){
    std::string fileName = getFileName(originalPath);
    size_t dotPos = fileName.find_last_of(".");
    if(dotPos != std::string::npos){
        std::string nameWithoutExt = fileName.substr(0, dotPos);
        std::string ext = fileName.substr(dotPos);
        return nameWithoutExt + "_copy" + ext;
    }
    else
    {
        return fileName + "_copy";
    }
}

bool checkContain(const std::string& fullPath){
    std::string searchPath = fullPath + "\\*";
    WIN32_FIND_DATA find;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &find);
   if(hFind == INVALID_HANDLE_VALUE){
        return false;  
    }
    do
    {
       if(strcmp(find.cFileName, ".") != 0 && 
            strcmp(find.cFileName, "..") != 0){
            FindClose(hFind);
            return true;  
        }
    } 
    while (FindNextFile(hFind, &find));
    FindClose(hFind);
    return false;  
}

bool createDirectory(const std::string& dirPath, const std::string& dirName){
    std::string Pfull = trimmed(dirPath) + "\\" + trimmed(dirName);
    
    bool isDir;
   if(!checkPath(trimmed(dirPath), isDir) || !isDir){
        std::cout<<"Parent directory not exist! "<<dirPath<<std::endl;
        return false;
    }
    
   if(checkPath(Pfull, isDir)){
       if(isDir){
            std::cout<<"Directory already exists!"<<std::endl;
            return false;
        }
    }
    
   if(CreateDirectory(Pfull.c_str(), NULL)){
        std::cout<<"Directory created. It's path: "<<Pfull<<std::endl;
        return true;
    }else{
        DWORD error = GetLastError();
       if(error == ERROR_ALREADY_EXISTS){
            std::cout<<"Error: Directory already exists!"<<std::endl;
        }else if(error == ERROR_PATH_NOT_FOUND){
            std::cout<<"Error: Parent directory does not exist!"<<std::endl;
        }else{
            std::cout<<"Error: failed to create directory! Error code: "<<error<<std::endl;
        }
        return false;
    }
}

bool removeDirectory(const std::string& dirPath){
    std::string path = trimmed(dirPath);
    std::cout<<"Remove directory: "<<path<<std::endl;
    bool isDir;
    if(!checkPath(path, isDir)){
            std::cout<<"Error/ Directory not exist"<<std::endl;
            return false;
        }
    if(!isDir){
            std::cout<<"Error.Path is not a directory"<<std::endl;
            return false;
        }
    if(checkContain(path)){
        std::cout<<"Error. Directory is not empty!"<<std::endl;
        return false;
        } 
    if(RemoveDirectory(path.c_str())){
            std::cout<<"Directory removed: "<<path;
            return true;
        }else{
            DWORD error = GetLastError();
        if(error == ERROR_DIR_NOT_EMPTY){
                std::cout<<"Error. Directory not empty!";
            }else if(error == ERROR_ACCESS_DENIED){
                std::cout<<"Error. Access denied";
            }else{
                std::cout<<"Error for removing directory. Code: "<<error;
            }
            return false;
        }
}

bool createFile(const std::string& dirPath, const std::string& fileName, const std::string& content = ""){
    std::string fullPath = trimmed(dirPath)+ "\\" + fileName;
    std::cout<<"Creating file: "<<fullPath<<std::endl;
    bool isDir;
    if(!checkPath(trimmed(dirPath), isDir)|| !isDir){
        std::cout<<"Error. Directory not exist: "<<trimmed(dirPath)<<std::endl;
        return false;
    }
    
    HANDLE hFile = CreateFile(
        fullPath.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if(hFile == INVALID_HANDLE_VALUE){
        std::cout<<"Error for create file!"<<std::endl;
        return false;
    }
    
    if(!content.empty()){
        DWORD bytesWritten;
        if(!WriteFile(hFile, content.c_str(), content.length(), &bytesWritten, NULL)){
            std::cout<<"Error to write to file"<<std::endl;
            CloseHandle(hFile);
            return false;
        }
        std::cout<<"Written "<<bytesWritten<<" bytes to file"<<std::endl;
    }
    
    CloseHandle(hFile);
    std::cout<<"File created. It's path: "<<fullPath<<std::endl;
    return true;
}

bool copyFile(const std::string& sourcePath, const std::string& destDir){
    bool isDir;

    if(!checkPath(trimmed(sourcePath), isDir)){
        std::cout<<"Error. directory "<<trimmed(sourcePath)<<" is invalid"<< std::endl;
        return false;

    }else if(!checkPath(trimmed(destDir), isDir)){
        std::cout<<"Error. directory "<<trimmed(destDir)<<" is invalid"<< std::endl;
        return false;
    }

    std::string copyFileName = getCopyFileName(trimmed(sourcePath));
    std::string destPath = trimmed(destDir)+ "\\" + copyFileName;
    
    bool destExists;
    bool destIsDir;
    destExists = checkPath(destPath, destIsDir);
    
    if(destExists && !destIsDir){
        std::cout<<"Overwrite (y/n): ";
        char choice;
        std::cin>>choice;
        if(tolower(choice)!= 'y'){
            std::cout<<"cancelled"<<std::endl;
            return false;
        }
    }
    
    if(CopyFile(trimmed(sourcePath).c_str(), destPath.c_str(), FALSE)){
        std::cout<<"File copied"<<std::endl;
        return true;
    }else{
        std::cout<<"Error. failed to copy file"<<std::endl;
        return false;
    }
}

bool moveFile(const std::string& sourcePath, const std::string& destPath){
    std::string src = trimmed(sourcePath);
    std::string dst = trimmed(destPath);
    bool isDir;
    if(!checkPath(src, isDir)){
            std::cout<<"Error. source file not exist"<<std::endl;
            return false;
        }  
    if(isDir){
            std::cout<<"Error. source is a directory!"<<std::endl;
            return false;
        } 
        std::string fileName = getFileName(src);
    if(checkPath(dst, isDir) && isDir){
            dst = dst + "\\" + fileName;
        } 
        bool destExists,destIsDir;
        char choice;
        destExists = checkPath(dst, destIsDir);
    if(destExists && !destIsDir){
            std::cout<<"file already exists"<<std::endl;
            std::cout<<"Overwrite? (y/n): ";
            std::cin>>choice;
        if(tolower(choice) != 'y'){
                std::cout<<"Move cancelled."<<std::endl;
                return false;
            }
        if(!DeleteFile(dst.c_str())){
                std::cout<<"Error to delete existing file"<<std::endl;
                return false;
            }
        }else if(destExists && destIsDir){
            std::cout<<"Error. destination is a directory"<<std::endl;
            return false;
        }
    if(MoveFile(src.c_str(), dst.c_str())){
            std::cout<<"file moved."<<std::endl;
            std::cout<<"from "<<src<<std::endl;
            std::cout<<"to "<<dst<<std::endl;
            return true;
        }
        else{
            DWORD error = GetLastError();
        if(error == ERROR_ACCESS_DENIED){
                std::cout<<"Error. Access denied"<<std::endl;
            }
            else if(error == ERROR_NOT_SAME_DEVICE){
                std::cout<<"Error. Cant move between different drives"<<std::endl;
            }
            else{
                std::cout<<"Error: failed to move file! Error code: "<<error<<std::endl;
            }
            return false;
        }
}
bool moveFileEx(const std::string& sourcePath, const std::string& destPath, DWORD flagss = 0){
    std::string src = trimmed(sourcePath);
    std::string dst = trimmed(destPath);
    bool isDir;
    if(!checkPath(src, isDir)){
        std::cout<<"Error. source file not exist"<<std::endl;
        return false;
    }
    if(isDir){
        std::cout<<"Error. source is a directory"<<std::endl;
        return false;
    }
    std::string fileName = getFileName(src);
    bool destExists = checkPath(dst, isDir);
    if(checkPath(dst, isDir) && isDir){
        dst = dst + "\\" + fileName;
        std::cout<<"Error. File will be moved as: "<<dst<<std::endl;
        destExists = checkPath(dst, isDir);
    }

    if(destExists && !isDir){
        if(!(flagss & MOVEFILE_REPLACE_EXISTING)){
            std::cout<<"Destination file exists"<<std::endl;
            std::cout<<"Overwrite? (y/n): ";
            char choice;
            std::cin>>choice;
            if(tolower(choice) != 'y'){
                std::cout<<"Move cancelled."<<std::endl;
                return false;
            }
            flagss |= MOVEFILE_REPLACE_EXISTING;
        }
    }


    if(MoveFileEx(src.c_str(), dst.c_str(), flagss)){
        std::cout<<"file moved"<<std::endl;
        std::cout<<"from "<<src<<std::endl;
        std::cout<<"to "<<dst<<std::endl;
        return true;
    }else{
        DWORD error = GetLastError();
        if(error == ERROR_ACCESS_DENIED){
            std::cout<<"Error.Try running as administrator."<<std::endl;
        }else if(error == ERROR_NOT_SAME_DEVICE){
            std::cout<<"Error. Cannot move between different drives"<<std::endl;
            std::cout<<"Use COPY_ALLOWED flag to enable cross-drive move."<<std::endl;
        }else{
            std::cout<<"Error: failed to move file! Code: "<<error<<std::endl;
        }
        return false;
    }
}

void caseGetLogicalDriveStrings(std::vector<std::string>& list){
    char lpBuffer[MAX_SIZE];
    int count = 1;
    list.clear();
    
    DWORD result = GetLogicalDriveStringsA(MAX_SIZE, lpBuffer);
    if((result > 0) && (result <= MAX_SIZE)){
        std::cout<<"List of drivers: "<<std::endl;
        char* drive = lpBuffer;
        while(*drive){
            std::cout<<count<<". "<<drive<<std::endl;
            list.push_back(std::string(drive));
            drive += strlen(drive) + 1;
            count++;
        }
    }else{
        std::cout<<"Error getting drive list"<<std::endl;
    }
}

void caseGetLogicalDrives(std::vector<std::string>& list){
    int count = 1;
    list.clear();
    DWORD drives = GetLogicalDrives();
    std::cout<<"List of drives: "<<std::endl;
    for(char d = 'A'; d <= 'Z'; d++){
        if(drives & 1){
            std::string disk = std::string(1, d) + ":\\";
            std::cout<<count<<". "<<disk<<std::endl;
            list.push_back(disk);
            count++;
        }
        drives >>= 1;
    }
}

void caseGetDriveType(const std::vector<std::string>& list){
    if(list.empty()){
        std::cout<<"Please check your drives first (Option 1-2 in menu)"<<std::endl; 
        return;
    }
    int id;
    std::cout<<"Select the driver please: ";
    std::cin >> id;
    id--;
    if(id < 0 || id >= (int)list.size()){
        std::cout<<"Invalid driver number!"<<std::endl;
        return;
    }
    UINT type = GetDriveTypeA(list[id].c_str());
    switch(type){
        case 0:
            std::cout<<"DRIVE_UNKNOWN (The drive type cannot be determined.)"<<std::endl;
            break;
        case 1:
            std::cout<<"DRIVE_NO_ROOT_DIR (The root path is invalid; for example, there is no volume mounted at the specified path.)"<<std::endl;
            break;
        case 2:
            std::cout<<"DRIVE_REMOVABLE (The drive has removable media; for example, a floppy drive, thumb drive, or flash card reader.)"<<std::endl;
            break;
        case 3:
            std::cout<<"DRIVE_FIXED (The drive has fixed media; for example, a hard disk drive or flash drive.)"<<std::endl;
            break;
        case 4:
            std::cout<<"DRIVE_REMOTE (The drive is a remote (network) drive.)"<<std::endl;
            break;
        case 5:
            std::cout<<"DRIVE_CDROM (The drive is a CD-ROM drive.)"<<std::endl;
            break;
        case 6:
            std::cout<<"DRIVE_RAMDISK (The drive is a RAM disk.)"<<std::endl;
            break;
        default:
            std::cout<<"Unknown drive type: "<<type<<std::endl;
            break;
    }
}

void caseGetVolumeInformation(const std::vector<std::string>& list){
    if(list.empty()){
        std::cout<<"First check your drivers (Option 1-2)"<<std::endl; 
        return;
    }
    CHAR VolumeBuff[MAX_SIZE], FileSysNameBuff[MAX_SIZE];
    DWORD componentlenght, flags;
    DWORD serial;
    
    int id;
    std::cout<<"Choose a number of driver: ";
    std::cin >> id;
    id--;
    if(id < 0 || id >= (int)list.size()){
        std::cout<<"Invalid number. Try again"<<std::endl;
        return;
    }
    
    BOOL info = GetVolumeInformationA(
        list[id].c_str(),
        VolumeBuff,
        MAX_SIZE,
        &serial,
        &componentlenght,
        &flags,
        FileSysNameBuff,
        MAX_SIZE
    );
    
    if(info){
        std::cout<<"Volume name: "<<VolumeBuff<<std::endl;
        std::cout<<"Serial number: "<<serial<<std::endl;
        std::cout<<"File system: "<<FileSysNameBuff<<std::endl;
        std::cout<<"Max length of name "<<componentlenght<<std::endl;
        std::cout<<"flags: "<<flags<<std::endl;

        if(flags & FILE_CASE_SENSITIVE_SEARCH) std::cout<<"  - Supports case-sensitive file names"<<std::endl;
        if(flags & FILE_CASE_PRESERVED_NAMES) std::cout<<"  - Preserves case of file names"<<std::endl;
        if(flags & FILE_UNICODE_ON_DISK) std::cout<<"  - Supports Unicode in file names"<<std::endl;
        if(flags & FILE_PERSISTENT_ACLS) std::cout<<"  - Preserves and enforces ACLs"<<std::endl;
        if(flags & FILE_FILE_COMPRESSION) std::cout<<"  - Supports file-based compression"<<std::endl;
        if(flags & FILE_VOLUME_QUOTAS) std::cout<<"  - Supports disk quotas"<<std::endl;
        if(flags & FILE_SUPPORTS_SPARSE_FILES) std::cout<<"  - Supports sparse files"<<std::endl;
        if(flags & FILE_SUPPORTS_REPARSE_POINTS) std::cout<<"  - Supports reparse points"<<std::endl;
        if(flags & FILE_SUPPORTS_REMOTE_STORAGE) std::cout<<"  - Supports remote storage"<<std::endl;
        if(flags & FILE_VOLUME_IS_COMPRESSED) std::cout<<"  - Volume is compressed"<<std::endl;
        if(flags & FILE_SUPPORTS_OBJECT_IDS) std::cout<<"  - Supports object identifiers"<<std::endl;
        if(flags & FILE_SUPPORTS_ENCRYPTION) std::cout<<"  - Supports Encrypted File System (EFS)"<<std::endl;
        if(flags & FILE_NAMED_STREAMS) std::cout<<"  - Supports named streams"<<std::endl;
        if(flags & FILE_READ_ONLY_VOLUME) std::cout<<"  - Volume is read-only"<<std::endl;
        if(flags & FILE_SUPPORTS_TRANSACTIONS) std::cout<<"  - Supports transactions"<<std::endl;
        if(flags & FILE_SUPPORTS_HARD_LINKS) std::cout<<"  - Supports hard links"<<std::endl;
        if(flags & FILE_SUPPORTS_EXTENDED_ATTRIBUTES) std::cout<<"  - Supports extended attributes"<<std::endl;
    }
    else{
        std::cout<<"Error of getting volume info. Code: "<<GetLastError()<<std::endl;
    }
}

void caseGetDiskFreeSpace(const std::vector<std::string>& list){
    if(list.empty()){
        std::cout<<"Please check your drives first (Option 1-2 in menu)"<<std::endl; 
        return;
    }
    DWORD SectorsPerCluster, BytesPerSector, NumberOfFreeClusters, TotalNumberOfClusters; 
    int id;
    std::cout<<"Please select a number of driver: ";
    std::cin >> id;
    id--;
    if(id < 0 || id >= (int)list.size()){
        std::cout<<"Invalid number"<<std::endl;
        return;
    }
    
    BOOL space = GetDiskFreeSpaceA(
        list[id].c_str(),
        &SectorsPerCluster,
        &BytesPerSector,
        &NumberOfFreeClusters,
        &TotalNumberOfClusters
    );
    
    if(space){
        __int64 totalBytes = (__int64)TotalNumberOfClusters * SectorsPerCluster * BytesPerSector;
        __int64 freeBytes = (__int64)NumberOfFreeClusters * SectorsPerCluster * BytesPerSector;
        std::cout<<"Bytes per sector: "<<BytesPerSector<<std::endl;
        std::cout<<"Sectors per cluster: "<<SectorsPerCluster<<std::endl;
        std::cout<<"Number of free clusters: "<<NumberOfFreeClusters<<std::endl;
        std::cout<<"Total clusters: "<<TotalNumberOfClusters<<std::endl;
        std::cout<<"\nTotal space: "<<totalBytes / (1024 * 1024)<<" MB ("<<totalBytes<<" bytes)"<<std::endl;
        std::cout<<"Free space: "<<freeBytes / (1024 * 1024)<<" MB ("<<freeBytes<<" bytes)"<<std::endl;
    }
    else{
        std::cout<<"Error for get free space. Code: "<<GetLastError()<<std::endl;
    }
}

void caseGetFileAttributes(const std::string& filePath){
    DWORD attribute = GetFileAttributesA(filePath.c_str());
    if(attribute == INVALID_FILE_ATTRIBUTES){
        std::cout<<"Error for get file attributes. Code: "<<GetLastError()<<std::endl;
        return;
    }
    
    std::cout<<"Attributes of file \""<<filePath<<"\":"<<std::endl;
    std::cout<<"----------------------------------------"<<std::endl;
    
    bool hasAttributes = false;
    
    if(attribute & FILE_ATTRIBUTE_DIRECTORY){
        std::cout<<"  - Directory"<<std::endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_READONLY){
        std::cout<<"  - Read-only"<<std::endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_HIDDEN){
        std::cout<<"  - Hidden"<<std::endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_SYSTEM){
        std::cout<<"  - System"<<std::endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_ARCHIVE){
        std::cout<<"  - Archive"<<std::endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_TEMPORARY){
        std::cout<<"  - Temporary"<<std::endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_NORMAL){
        std::cout<<"  - Normal file"<<std::endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_COMPRESSED){
        std::cout<<"  - Compressed"<<std::endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_OFFLINE){
        std::cout<<"  - Offline"<<std::endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED){
        std::cout<<"  - Not content indexed"<<std::endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_ENCRYPTED){
        std::cout<<"  - Encrypted"<<std::endl;
        hasAttributes = true;
    }
    
    if(!hasAttributes){
        std::cout<<"  - No special attributes"<<std::endl;
    }
    
    std::cout<<"----------------------------------------"<<std::endl;
    std::cout<<"Attribute value (hex): 0x"<<std::hex<<attribute<<std::dec<<std::endl;
}

void caseSetFileAttributes(const std::string& filePath){
    int indexA;
    bool bAttr;
    DWORD nAttribute;
    
    std::cout<<"\n=== Set File Attributes ==="<<std::endl;
    std::cout<<"Current attributes:"<<std::endl;
    caseGetFileAttributes(filePath);
    
    std::cout<<"\n=== Choose attribute to set ==="<<std::endl;
    std::cout<<"1.  ARCHIVE"<<std::endl;
    std::cout<<"2.  HIDDEN"<<std::endl;
    std::cout<<"3.  NORMAL (clears all other attributes)"<<std::endl;
    std::cout<<"4.  NOT CONTENT INDEXED"<<std::endl;
    std::cout<<"5.  OFFLINE"<<std::endl;
    std::cout<<"6.  READONLY"<<std::endl;
    std::cout<<"7.  SYSTEM"<<std::endl;
    std::cout<<"8.  TEMPORARY"<<std::endl;
    std::cout<<"9.  COMPRESSED"<<std::endl;
    std::cout<<"10. ENCRYPTED"<<std::endl;
    std::cout<<"0.  Cancel"<<std::endl;
    std::cout<<"Choose attribute: ";
    std::cin >> indexA;
    
    switch(indexA){
        case 1:
            nAttribute = FILE_ATTRIBUTE_ARCHIVE;
            break;
        case 2:
            nAttribute = FILE_ATTRIBUTE_HIDDEN;
            break;
        case 3:
            nAttribute = FILE_ATTRIBUTE_NORMAL;
            break;
        case 4:
            nAttribute = FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
            break;
        case 5:
            nAttribute = FILE_ATTRIBUTE_OFFLINE;
            break;
        case 6:
            nAttribute = FILE_ATTRIBUTE_READONLY;
            break;
        case 7:
            nAttribute = FILE_ATTRIBUTE_SYSTEM;
            break;
        case 8:
            nAttribute = FILE_ATTRIBUTE_TEMPORARY;
            break;
        case 9:
            nAttribute = FILE_ATTRIBUTE_COMPRESSED;
            break;
        case 10:
            nAttribute = FILE_ATTRIBUTE_ENCRYPTED;
            break;
        case 0:
            std::cout<<"Operation cancelled."<<std::endl;
            return;
        default:
            std::cout<<"Invalid choice!"<<std::endl;
            return;
    }
    
    bAttr = SetFileAttributesA(filePath.c_str(), nAttribute);
    if(bAttr){
        std::cout<<"Attributes changed"<<std::endl;
        std::cout<<"New attributes:"<<std::endl;
        caseGetFileAttributes(filePath);
    }
    else{
        std::cout<<"Error for setting attributes. Code: "<<GetLastError()<<std::endl;
        if(GetLastError() == ERROR_ACCESS_DENIED){
            std::cout<<"Error. Run as administrator."<<std::endl;
        }
    }
}

void caseGetFileInformationByHandle(const std::string& filePath){
    ULONGLONG fileSize, fileId;
    BY_HANDLE_FILE_INFORMATION lpFileInformation;
    
    HANDLE hInfFile = CreateFile(
        filePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if(hInfFile == INVALID_HANDLE_VALUE){
        std::cout<<"Error opening file. Code: "<<GetLastError()<<std::endl;
        return;
    }
    
    if(GetFileInformationByHandle(hInfFile, &lpFileInformation)){
        std::cout<<"\n=== File Information ==="<<std::endl;
        std::cout<<"File: "<<filePath<<std::endl;
        std::cout<<"----------------------------------------"<<std::endl;
        std::cout<<"Attributes: ";
        if(lpFileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) std::cout<<"Directory ";
        if(lpFileInformation.dwFileAttributes & FILE_ATTRIBUTE_READONLY) std::cout<<"Read only ";
        if(lpFileInformation.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) std::cout<<"Hidden ";
        if(lpFileInformation.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) std::cout<<"System ";
        if(lpFileInformation.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) std::cout<<"Archive ";
        if(lpFileInformation.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) std::cout<<"Temporary ";
        std::cout<<std::endl;

        printFileTimeH(lpFileInformation.ftCreationTime, "Created");
        printFileTimeH(lpFileInformation.ftLastAccessTime, "Last access");
        printFileTimeH(lpFileInformation.ftLastWriteTime, "Last modified");
        
        fileSize = (static_cast<ULONGLONG>(lpFileInformation.nFileSizeHigh)<<32) | lpFileInformation.nFileSizeLow;
        std::cout<<"\nSize: "<<fileSize<<" bytes";
        if(fileSize > 1024) std::cout<<" ("<<std::fixed<<std::setprecision(2)<<(double)fileSize / 1024<<" KB)";
        if(fileSize > 1024 * 1024) std::cout<<" ("<<std::fixed<<std::setprecision(2)<<(double)fileSize / (1024 * 1024)<<" MB)";
        std::cout<<std::endl;
        std::cout<<"Volume serial number: 0x"<<std::hex<<lpFileInformation.dwVolumeSerialNumber<<std::dec<<std::endl;
        std::cout<<"Number of hard links: "<<lpFileInformation.nNumberOfLinks<<std::endl;
        if(lpFileInformation.nNumberOfLinks > 1){
            std::cout<<"  (This file has multiple hard links)"<<std::endl;
        }

        fileId = (static_cast<ULONGLONG>(lpFileInformation.nFileIndexHigh)<<32) | lpFileInformation.nFileIndexLow;
        std::cout<<"Unique file ID: 0x"<<std::hex<<fileId<<std::dec<<std::endl;
        std::cout<<"----------------------------------------"<<std::endl;
    }
    else{
        std::cout<<"Error getting file information! Error code: "<<GetLastError()<<std::endl;
    }
    
    CloseHandle(hInfFile);
}

void caseGetFileTime(const std::string& filePath){
    FILETIME createTime, accessTime, writeTime;
    
    HANDLE hFile = CreateFile(
        filePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if(hFile == INVALID_HANDLE_VALUE){
        std::cout<<"Error opening file! Error code: "<<GetLastError()<<std::endl;
        return;
    } 
    if(GetFileTime(hFile, &createTime, &accessTime, &writeTime)){
        std::cout<<"\n=== File Time Information ==="<<std::endl;
        std::cout<<"File: "<<filePath<<std::endl;
        std::cout<<"----------------------------------------"<<std::endl;
        printFileTimeT(createTime, "Created");
        printFileTimeT(accessTime, "Last accessed");
        printFileTimeT(writeTime, "Last modified");
        std::cout<<"----------------------------------------"<<std::endl;
    }
    else{
        std::cout<<"Failed to get file time! Error code: "<<GetLastError()<<std::endl;
    }
    
    CloseHandle(hFile);
}

void caseSetFileTime(const std::string& filePath){
    int ID, year, month, day, hour, minute, second;
    DWORD flags, attrs;
    HANDLE hFile;
    FILETIME verifyCreate, verifyAccess, verifyWrite;
    
    attrs = GetFileAttributesA(filePath.c_str());
    if(attrs == INVALID_FILE_ATTRIBUTES){
        std::cout<<"Error. File not exist. Code: "<<GetLastError()<<std::endl;
        return;
    }
    
    std::cout<<"\n=== Current File Times ==="<<std::endl;
    caseGetFileTime(filePath);
    
    std::cout<<"\n=== Select time mode ==="<<std::endl;
    std::cout<<"1. Set to current system time"<<std::endl;
    std::cout<<"2. Set custom date and time"<<std::endl;
    std::cout<<"3. Set creation time only"<<std::endl;
    std::cout<<"4. Set last access time only"<<std::endl;
    std::cout<<"5. Set last modified time only"<<std::endl;
    std::cout<<"Choose (1-5): ";
    std::cin >> ID;
    
    flags = FILE_ATTRIBUTE_NORMAL;
    if(attrs & FILE_ATTRIBUTE_DIRECTORY){
        flags = FILE_FLAG_BACKUP_SEMANTICS;
        std::cout<<"Target is a directory"<<std::endl;
    }
    
    hFile = CreateFile(
        filePath.c_str(),
        FILE_WRITE_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        flags,
        NULL
    );
    
    if(hFile == INVALID_HANDLE_VALUE){
        std::cout<<"Error opening file. Code: "<<GetLastError()<<std::endl;
        return;
    }
    
    SYSTEMTIME st = {0};
    FILETIME newTime, newCreateTime, newAccessTime, newWriteTime;
    BOOL success = FALSE;
    
    switch (ID){
        case 1: 
        {
            GetSystemTime(&st);
            SystemTimeToFileTime(&st, &newTime);
            std::cout<<"Setting all file times to current system time..."<<std::endl;
            success = SetFileTime(hFile, &newTime, &newTime, &newTime);
        }
        break;
        
        case 2: 
        {
            std::cout<<"\nEnter custom date and time:"<<std::endl;
            std::cout<<"Year: ";
            std::cin >> year;
            std::cout<<"Month (1-12): ";
            std::cin >> month;
            std::cout<<"Day (1-31): ";
            std::cin >> day;
            std::cout<<"Hour (0-23): ";
            std::cin >> hour;
            std::cout<<"Minute (0-59): ";
            std::cin >> minute;
            std::cout<<"Second (0-59): ";
            std::cin >> second;
            
            st.wYear = year;
            st.wMonth = month;
            st.wDay = day;
            st.wHour = hour;
            st.wMinute = minute;
            st.wSecond = second;
            st.wMilliseconds = 0;
            
            if(st.wYear < 1601 || st.wMonth < 1 || st.wMonth > 12 || 
                st.wDay < 1 || st.wDay > 31 || st.wHour > 23 || 
                st.wMinute > 59 || st.wSecond > 59){
                std::cout<<"Error: Invalid date or time!"<<std::endl;
                CloseHandle(hFile);
                return;
            }
            
            if(!SystemTimeToFileTime(&st, &newTime)){
                std::cout<<"Error converting time. Code: "<<GetLastError()<<std::endl;
                CloseHandle(hFile);
                return;
            }
            
            std::cout<<"Setting file date to custom"<<std::endl;
            success = SetFileTime(hFile, &newTime, &newTime, &newTime);
        }
        break;
        
        case 3: 
        {
            std::cout<<"\nEnter custom date and time:"<<std::endl;
            std::cout<<"Year: ";
            std::cin >> year;
            std::cout<<"Month (1-12): ";
            std::cin >> month;
            std::cout<<"Day (1-31): ";
            std::cin >> day;
            std::cout<<"Hour (0-23): ";
            std::cin >> hour;
            std::cout<<"Minute (0-59): ";
            std::cin >> minute;
            std::cout<<"Second (0-59): ";
            std::cin >> second;
            
            st.wYear = year;
            st.wMonth = month;
            st.wDay = day;
            st.wHour = hour;
            st.wMinute = minute;
            st.wSecond = second;
            st.wMilliseconds = 0;
            
            if(st.wYear < 1601 || st.wMonth < 1 || st.wMonth > 12 || 
                st.wDay < 1 || st.wDay > 31 || st.wHour > 23 || 
                st.wMinute > 59 || st.wSecond > 59){
                std::cout<<"Error. Invalid data"<<std::endl;
                CloseHandle(hFile);
                return;
            }
            
            if(!SystemTimeToFileTime(&st, &newCreateTime)){
                std::cout<<"Error converting time. cCcode: "<<GetLastError()<<std::endl;
                CloseHandle(hFile);
                return;
            }
            
            std::cout<<"Setting creation time only"<<std::endl;
            success = SetFileTime(hFile, &newCreateTime, NULL, NULL);
        }
        break;
        
        case 4: 
        {
            std::cout<<"\nEnter custom last access date and time:"<<std::endl;
            std::cout<<"Year: ";
            std::cin >> year;
            std::cout<<"Month (1-12): ";
            std::cin >> month;
            std::cout<<"Day (1-31): ";
            std::cin >> day;
            std::cout<<"Hour (0-23): ";
            std::cin >> hour;
            std::cout<<"Minute (0-59): ";
            std::cin >> minute;
            std::cout<<"Second (0-59): ";
            std::cin >> second;
            
            st.wYear = year;
            st.wMonth = month;
            st.wDay = day;
            st.wHour = hour;
            st.wMinute = minute;
            st.wSecond = second;
            st.wMilliseconds = 0;
            
            if(st.wYear < 1601 || st.wMonth < 1 || st.wMonth > 12 || 
                st.wDay < 1 || st.wDay > 31 || st.wHour > 23 || 
                st.wMinute > 59 || st.wSecond > 59){
                std::cout<<"Error. Invalid date or time!"<<std::endl;
                CloseHandle(hFile);
                return;
            }
            
            if(!SystemTimeToFileTime(&st, &newAccessTime)){
                std::cout<<"Error converting time. Code: "<<GetLastError()<<std::endl;
                CloseHandle(hFile);
                return;
            }
            
            std::cout<<"Setting last access time only"<<std::endl;
            success = SetFileTime(hFile, NULL, &newAccessTime, NULL);
        }
        break;
        
        case 5: 
        {
            std::cout<<"\nEnter custom last modified date and time:"<<std::endl;
            std::cout<<"Year: ";
            std::cin >> year;
            std::cout<<"Month (1-12): ";
            std::cin >> month;
            std::cout<<"Day (1-31): ";
            std::cin >> day;
            std::cout<<"Hour (0-23): ";
            std::cin >> hour;
            std::cout<<"Minute (0-59): ";
            std::cin >> minute;
            std::cout<<"Second (0-59): ";
            std::cin >> second;
            
            st.wYear = year;
            st.wMonth = month;
            st.wDay = day;
            st.wHour = hour;
            st.wMinute = minute;
            st.wSecond = second;
            st.wMilliseconds = 0;
            
            if(st.wYear < 1601 || st.wMonth < 1 || st.wMonth > 12 || 
                st.wDay < 1 || st.wDay > 31 || st.wHour > 23 || 
                st.wMinute > 59 || st.wSecond > 59){
                std::cout<<"Error: Invalid date or time!"<<std::endl;
                CloseHandle(hFile);
                return;
            }
            
            if(!SystemTimeToFileTime(&st, &newWriteTime)){
                std::cout<<"Error converting time. Сode: "<<GetLastError()<<std::endl;
                CloseHandle(hFile);
                return;
            }
            
            std::cout<<"Setting last modified time only"<<std::endl;
            success = SetFileTime(hFile, NULL, NULL, &newWriteTime);
        }
        break;
        
        default:
            std::cout<<"Invalid choice! Only(1-5)"<<std::endl;
            CloseHandle(hFile);
            return;
    }
    
    if(success){



        std::cout<<"\nFile time updated"<<std::endl;
        std::cout<<"\n=== New File Times ==="<<std::endl;
        if(GetFileTime(hFile, &verifyCreate, &verifyAccess, &verifyWrite)){
           printFileTimeT(verifyCreate, "Created");
           printFileTimeT(verifyAccess, "Last accessed");
           printFileTimeT(verifyWrite, "Last modified");
        }
    }else{
        std::cout<<"Error setting file times. Code: "<<GetLastError()<<std::endl;
        if(GetLastError() == ERROR_ACCESS_DENIED){
            std::cout<<"Error. Try run as administrator."<<std::endl;
        }
    }

    CloseHandle(hFile);
}

int main()
{   
    int flag = 1; 
    int menu, key;
    std::vector<std::string> buffForDrivers;
    std::string pathFile,directoryDest,sourcePath,data,fileName,directoryPath,destPath,directoryName;

    
    while(flag){

        std::cout<<"\n=====Menu====="<<std::endl;
        std::cout<<"\n1.Show list of drivers (GetLogicalDrives)"<<std::endl;
        std::cout<<"2.Show list of drivers (GetLogicalDrivesStrings)"<<std::endl;
        std::cout<<"3.Driver Type (GetDriverType)"<<std::endl;
        std::cout<<"4.Driver volume info (GetVolumeInformation)"<<std::endl;
        std::cout<<"5.Driver space (GetDiskFreeSpace)"<<std::endl;
        std::cout<<"6.Create directory (CreateDirectory)"<<std::endl;
        std::cout<<"7.Remove directory (RemoveDirectory)"<<std::endl;
        std::cout<<"8.Create file (CreateFile)"<<std::endl;
        std::cout<<"9.Copy file (CopyFile)"<<std::endl;
        std::cout<<"10.Move file (MoveFile)"<<std::endl;
        std::cout<<"11.Move file (MoveFileEx)"<<std::endl;
        std::cout<<"12.File Attribute (GetFileAttributes)"<<std::endl;
        std::cout<<"13.Set Attribute (SetFileAttributes)"<<std::endl;
        std::cout<<"14.Show file information (GetFileInformationByHandle)"<<std::endl;
        std::cout<<"15.Get file time (GetFileTime)"<<std::endl;
        std::cout<<"16.Set file time (SetFileTime)"<<std::endl;
        std::cout<<"0.Exit"<<std::endl;
        std::cout<<"\n=Choose function: ";
        std::cin>>menu;
        std::cout<<"\n";
        
        switch(menu){
            case 1:  
                caseGetLogicalDrives(buffForDrivers);
                break;
            case 2:
                caseGetLogicalDriveStrings(buffForDrivers);
                break;
            case 3:
                caseGetDriveType(buffForDrivers);
                break;
            case 4:
                caseGetVolumeInformation(buffForDrivers);
                break;
            case 5:
                caseGetDiskFreeSpace(buffForDrivers);
                break;
            case 6:
            {
                std::cin.ignore();
                std::cout<<"Enter path of directory: ";
                getline(std::cin, directoryPath);
                std::cout<<"Enter name: ";
                getline(std::cin, directoryName);
                createDirectory(directoryPath, directoryName);
                break;

            }
            case 7:
            {
                std::cin.ignore();
                std::cout<<"Enter path of directory: ";
                getline(std::cin, directoryPath);
                removeDirectory(directoryPath);
                break;
            }
            case 8:
            {
                std::cin.ignore();
                std::cout<<"Enter path of directory: ";
                getline(std::cin, directoryPath);
                std::cout<<"Enter name: ";
                getline(std::cin, fileName);
                std::cout<<"Enter file content (Enter for empty): ";
                getline(std::cin, data);
                createFile(directoryPath, fileName, data);
                break;
            }
            case 9: 
            {
                std::cin.ignore();
                std::cout<<"Enter source path: ";
                getline(std::cin, sourcePath);
                std::cout<<"Enter directory: ";
                getline(std::cin, directoryDest);
                copyFile(sourcePath, directoryDest);
                break;
            }
            case 10:
            {
                std::cin.ignore();
                std::cout<<"Enter source file path: ";
                getline(std::cin, sourcePath);
                std::cout<<"Enter destination (file or directory): ";
                getline(std::cin, destPath);
                moveFile(sourcePath, destPath);
                break;
            }
            case 11: 
            {
                std::cin.ignore();
                std::cout<<"Enter source file path: ";
                getline(std::cin, sourcePath);
                std::cout<<"Enter destination (file or directory): ";
                getline(std::cin, destPath);
                
                std::cout<<"\nSelect flags:"<<std::endl;
                std::cout<<"1.Normal"<<std::endl;
                std::cout<<"2.Replace exist file"<<std::endl;
                std::cout<<"3.Replace and Allow copy"<<std::endl;
                std::cout<<"4.Allow copy between different drives"<<std::endl;
                std::cin>>key;
                
                DWORD flags = 0;
                switch(key){
                    case 1: 
                        flags = 0;
                        break;
                    case 2:
                        flags = MOVEFILE_REPLACE_EXISTING;
                        break;
                    case 3: 
                        flags = MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED;
                        break;
                    case 4:
                        flags = MOVEFILE_COPY_ALLOWED;
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
                std::cin.ignore();
                std::cout<<"Enter path of your file ";
                std::getline(std::cin, pathFile);
                caseGetFileAttributes(pathFile);
                break;
            }
            case 13:
            {
                std::cin.ignore();
                std::cout<<"Enter path of your file ";
                std::getline(std::cin, pathFile);
                caseSetFileAttributes(pathFile);
                break;
            }
            case 14:
            {
                std::cin.ignore();
                std::cout<<"Enter path of your file ";
                getline(std::cin, pathFile);
                caseGetFileInformationByHandle(pathFile);
                break;
            }
            case 15:
            {
                std::cin.ignore();
                std::cout<<"Enter path of your file ";
                getline(std::cin, pathFile);
                caseGetFileTime(pathFile);
                break;
            }
            case 16:
            {
                std::cin.ignore();
                std::cout<<"Enter path of your file ";
                getline(std::cin, pathFile);
                caseSetFileTime(pathFile);
                break;
            }
            case 0:
                flag = 0;
                break;
        }
    }
    return 0;
}