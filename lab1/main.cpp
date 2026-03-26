#include <iostream>
#include <windows.h>
#include <vector>
#include <string.h>
#include <iomanip>
#define MAX_SIZE 250

using namespace std;

string trimmed(const string& str){
    size_t first = str.find_first_not_of(" \t\n\r");
    if(first == string::npos)return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

void printFileTimeForTime(const FILETIME& ftime,const string& labe){
        if(ftime.dwHighDateTime == 0 && ftime.dwLowDateTime ==0){
            cout<<labe<<": Not avalible"<<endl;
            return;
        }
        SYSTEMTIME stime;
        FileTimeToSystemTime(&ftime,&stime);

        cout<<labe<<": "<<std::setfill('0')<<std::setw(2)<<stime.wDay<<"."<<std::setw(2)<<stime.wMonth<<"."<<std::setw(4)<<stime.wYear<<" "<<std::setw(2)<<stime.wHour<<":"<<std::setw(2)<<stime.wMinute<<":"<<std::setw(2)<<stime.wSecond<<std::setfill(' ')<<endl;
    }

void printFileTimeForHandle(const FILETIME& ft, const char* label){
	SYSTEMTIME st;
	FileTimeToSystemTime(&ft, &st);
	std::cout<<label<<": "<<std::setfill('0')<< st.wDay<<"."<<std::setw(2)<<st.wMonth<<"."<<st.wYear<<" "<< std::setw(2)<<st.wHour<<":"<<std::setw(2)<<st.wMinute<<":"<<std::setw(2)<<st.wSecond<<std::endl;
}


bool checkPath(const string& path, bool& isDirectory){
    DWORD attrs = GetFileAttributes(path.c_str());
    if(attrs == INVALID_FILE_ATTRIBUTES){
        return false;
    }
    isDirectory = (attrs & FILE_ATTRIBUTE_DIRECTORY)!= 0;
    return true;
}

string getFileName(const string& path){
    size_t pos = path.find_last_of("\\");
    if(pos != string::npos){
        return path.substr(pos + 1);
    }
    return path;
}

string getCopyFileName(const string& originalPath){
    string fileName = getFileName(originalPath);
    size_t dotPos = fileName.find_last_of(".");
    
    if(dotPos != string::npos){
        string nameWithoutExt = fileName.substr(0, dotPos);
        string ext = fileName.substr(dotPos);
        return nameWithoutExt + "_copy" + ext;
    }else{
        return fileName + "_copy";
    }
}

bool checkContain(const string& fullPath){
    string searchPath = fullPath + "\\*";
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);
    
   if(hFind == INVALID_HANDLE_VALUE){
        return false;  
    }
    
    do{
       if(strcmp(findData.cFileName, ".") != 0 && 
            strcmp(findData.cFileName, "..") != 0){
            FindClose(hFind);
            return true;  
        }
    } while (FindNextFile(hFind, &findData));
    
    FindClose(hFind);
    return false;  
}

bool createDirectory(const string& dirPath, const string& dirName){
    string fullPath = trimmed(dirPath) + "\\" + trimmed(dirName);
    
    bool isDir;
   if(!checkPath(trimmed(dirPath), isDir) || !isDir){
        cout<<"Error: Parent directory does not exist: "<<dirPath<<endl;
        return false;
    }
    
   if(checkPath(fullPath, isDir)){
       if(isDir){
            cout<<"Warning: Directory already exists!"<<endl;
            return false;
        }
    }
    
   if(CreateDirectory(fullPath.c_str(), NULL)){
        cout<<"Directory successfully created: "<<fullPath<<endl;
        return true;
    }else{
        DWORD error = GetLastError();
       if(error == ERROR_ALREADY_EXISTS){
            cout<<"Error: Directory already exists!"<<endl;
        }else if(error == ERROR_PATH_NOT_FOUND){
            cout<<"Error: Parent directory does not exist!"<<endl;
        }else{
            cout<<"Error: failed to create directory! Error code: "<<error<<endl;
        }
        return false;
    }
}

bool removeDirectory(const string& dirPath){
    string path =trimmed(dirPath);
    
    cout<<"Removing directory: "<<path<<endl;
    
    bool isDir;
    if(!checkPath(path, isDir)){
            cout<<"Error: Directory does not exist!"<<endl;
            return false;
        }
    
    if(!isDir){
            cout<<"Error: Path is not a directory!"<<endl;
            return false;
        }
        
    if(checkContain(path)){
        cout<<"Error: Directory is not empty! Please delete all files first."<<endl;
        return false;
        }
        
    if(RemoveDirectory(path.c_str())){
            cout<<"Directory successfully removed: "<<path<<endl;
            return true;
        }else{
            DWORD error = GetLastError();
        if(error == ERROR_DIR_NOT_EMPTY){
                cout<<"Error: Directory is not empty!"<<endl;
            }else if(error == ERROR_ACCESS_DENIED){
                cout<<"Error: Access denied!"<<endl;
            }else{
                cout<<"Error: failed to remove directory! Error code: "<<error<<endl;
            }
            return false;
        }
}

bool createFile(const string& dirPath, const string& fileName, const string& content = ""){
    string fullPath = trimmed(dirPath)+ "\\" + fileName;
    
    cout<<"Creating file: "<<fullPath<<endl;
    
    bool isDir;
    if(!checkPath(trimmed(dirPath), isDir)|| !isDir){
        cout<<"Error: directory does not exist: "<<trimmed(dirPath)<<endl;
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
        cout<<"Error: failed to create file!"<<endl;
        return false;
    }
    
    if(!content.empty()){
        DWORD bytesWritten;
        if(!WriteFile(hFile, content.c_str(), content.length(), &bytesWritten, NULL)){
            cout<<"Error: failed to write to file!"<<endl;
            CloseHandle(hFile);
            return false;
        }
        cout<<"Written "<<bytesWritten<<" bytes to file"<<endl;
    }
    
    CloseHandle(hFile);
    cout<<"File successfully created: "<<fullPath<<endl;
    return true;
}

bool copyFile(const string& sourcePath, const string& destDir){
    bool isDir;
    if(!checkPath(trimmed(sourcePath), isDir)){
        cout<<"Error: directory "<<trimmed(sourcePath)<<" is incorrect"<< endl;
        return false;
    }else if(!checkPath(trimmed(destDir), isDir)){
        cout<<"Error: directory "<<trimmed(destDir)<<" is incorrect"<< endl;
        return false;
    }

    string copyFileName = getCopyFileName(trimmed(sourcePath));
    string destPath = trimmed(destDir)+ "\\" + copyFileName;
    
    bool destExists;
    bool destIsDir;
    destExists = checkPath(destPath, destIsDir);
    
    if(destExists && !destIsDir){
        cout<<"File already exists! Overwrite? (y/n): ";
        char choice;
        cin>>choice;
        if(tolower(choice)!= 'y'){
            cout<<"Copy cancelled."<<endl;
            return false;
        }
    }
    
    if(CopyFile(trimmed(sourcePath).c_str(), destPath.c_str(), FALSE)){
        cout<<"File successfully copied!"<<endl;
        return true;
    }else{
        cout<<"Error: failed to copy file!"<<endl;
        return false;
    }
}

bool moveFile(const string& sourcePath, const string& destPath){
    string src = trimmed(sourcePath);
    string dst = trimmed(destPath);
    
    bool isDir;
    if(!checkPath(src, isDir)){
            cout<<"Error: source file does not exist!"<<endl;
            return false;
        }
        
    if(isDir){
            cout<<"Error: source is a directory!"<<endl;
            return false;
        }
        
        string fileName = getFileName(src);
        
    if(checkPath(dst, isDir) && isDir){
            dst = dst + "\\" + fileName;
        }
        
        bool destExists;
        bool destIsDir;
        destExists = checkPath(dst, destIsDir);
        
    if(destExists && !destIsDir){
            cout<<"Destination file already exists!"<<endl;
            cout<<"Overwrite? (y/n): ";
            char choice;
            cin>>choice;
        if(tolower(choice) != 'y'){
                cout<<"Move cancelled."<<endl;
                return false;
            }
        if(!DeleteFile(dst.c_str())){
                cout<<"Error: failed to delete existing file!"<<endl;
                return false;
            }
        }else if(destExists && destIsDir){
            cout<<"Error: destination is a directory! Please specify a file name."<<endl;
            return false;
        }
        
    if(MoveFile(src.c_str(), dst.c_str())){
            cout<<"File successfully moved!"<<endl;
            cout<<"From: "<<src<<endl;
            cout<<"To: "<<dst<<endl;
            return true;
        }else{
            DWORD error = GetLastError();
        if(error == ERROR_ACCESS_DENIED){
                cout<<"Error: Access denied!"<<endl;
            }else if(error == ERROR_NOT_SAME_DEVICE){
                cout<<"Error: Cannot move between different drives!"<<endl;
            }else{
                cout<<"Error: failed to move file! Error code: "<<error<<endl;
            }
            return false;
        }
}
bool moveFileEx(const string& sourcePath, const string& destPath, DWORD flags = 0){
    string src = trimmed(sourcePath);
    string dst = trimmed(destPath);
    
    bool isDir;
    if(!checkPath(src, isDir)){
        cout<<"Error: source file does not exist!"<<endl;
        return false;
    }
    
    if(isDir){
        cout<<"Error: source is a directory!"<<endl;
        return false;
    }
    
    string fileName = getFileName(src);
    
    bool destExists = checkPath(dst, isDir);
    
    if(checkPath(dst, isDir) && isDir){
        dst = dst + "\\" + fileName;
        cout<<"Destination is a directory. File will be moved as: "<<dst<<endl;
        destExists = checkPath(dst, isDir);
    }

    
    
    if(destExists && !isDir){
        if(!(flags & MOVEFILE_REPLACE_EXISTING)){
            cout<<"Warning: Destination file already exists!"<<endl;
            cout<<"Overwrite? (y/n): ";
            char choice;
            cin>>choice;
            if(tolower(choice) != 'y'){
                cout<<"Move cancelled."<<endl;
                return false;
            }
            flags |= MOVEFILE_REPLACE_EXISTING;
        }
    }
    
    if(MoveFileEx(src.c_str(), dst.c_str(), flags)){
        cout<<"File successfully moved!"<<endl;
        cout<<"From: "<<src<<endl;
        cout<<"To: "<<dst<<endl;
        return true;
    }else{
        DWORD error = GetLastError();
        if(error == ERROR_ACCESS_DENIED){
            cout<<"Error: Access denied! Try running as administrator."<<endl;
        }else if(error == ERROR_NOT_SAME_DEVICE){
            cout<<"Error: Cannot move between different drives!"<<endl;
            cout<<"Use COPY_ALLOWED flag to enable cross-drive move."<<endl;
        }else{
            cout<<"Error: failed to move file! Error code: "<<error<<endl;
        }
        return false;
    }
}

void showDrivesByGetLogicalDrivestrings(vector<string>& listofdrivers){
    char lpBuffer[MAX_SIZE];
    int count = 1;
    listofdrivers.clear();
    
    DWORD result = GetLogicalDriveStringsA(MAX_SIZE, lpBuffer);
    if((result > 0) && (result <= MAX_SIZE)){
        cout<<"Your drives: "<<endl;
        char* drive = lpBuffer;
        while(*drive){
            cout<<count<<". "<<drive<<endl;
            listofdrivers.push_back(string(drive));
            drive += strlen(drive) + 1;
            count++;
        }
    }else{
        cout<<"Error getting drive list!"<<endl;
    }
}

void showDrivesByGetLogicalDrives(vector<string>& listofdrivers){
    int count = 1;
    listofdrivers.clear();
    DWORD drives = GetLogicalDrives();
    cout<<"Your drives: "<<endl;
    for(char d = 'A'; d <= 'Z'; d++){
        if(drives & 1){
            string disk = string(1, d) + ":\\";
            cout<<count<<". "<<disk<<endl;
            listofdrivers.push_back(disk);
            count++;
        }
        drives >>= 1;
    }
}

void showDriveType(const vector<string>& listofdrivers){
    if(listofdrivers.empty()){
        cout<<"Please check your drives first (Option 1-2 in menu)"<<endl; 
        return;
    }
    
    int id;
    cout<<"Please select a disk number: ";
    cin >> id;
    
    if(id < 0 || id >= (int)listofdrivers.size()){
        cout<<"Invalid disk number!"<<endl;
        return;
    }
    
    UINT type = GetDriveTypeA(listofdrivers[id].c_str());
    switch(type){
        case 0:
            cout<<"DRIVE_UNKNOWN (The drive type cannot be determined.)"<<endl;
            break;
        case 1:
            cout<<"DRIVE_NO_ROOT_DIR (The root path is invalid; for example, there is no volume mounted at the specified path.)"<<endl;
            break;
        case 2:
            cout<<"DRIVE_REMOVABLE (The drive has removable media; for example, a floppy drive, thumb drive, or flash card reader.)"<<endl;
            break;
        case 3:
            cout<<"DRIVE_FIXED (The drive has fixed media; for example, a hard disk drive or flash drive.)"<<endl;
            break;
        case 4:
            cout<<"DRIVE_REMOTE (The drive is a remote (network) drive.)"<<endl;
            break;
        case 5:
            cout<<"DRIVE_CDROM (The drive is a CD-ROM drive.)"<<endl;
            break;
        case 6:
            cout<<"DRIVE_RAMDISK (The drive is a RAM disk.)"<<endl;
            break;
        default:
            cout<<"Unknown drive type: "<<type<<endl;
            break;
    }
}

void showVolumeInformation(const vector<string>& listofdrivers){
    if(listofdrivers.empty()){
        cout<<"Please check your drives first (Option 1-2 in menu)"<<endl; 
        return;
    }
    
    CHAR VolumeNameBuffer[MAX_SIZE];
    CHAR FileSystemNameBuffer[MAX_SIZE];
    DWORD componentlenght, flags;
    DWORD serial;
    
    int id;
    cout<<"Please select a disk number: ";
    cin >> id;
    
    if(id < 0 || id >= (int)listofdrivers.size()){
        cout<<"Invalid disk number!"<<endl;
        return;
    }
    
    BOOL info = GetVolumeInformationA(
        listofdrivers[id].c_str(),
        VolumeNameBuffer,
        MAX_SIZE,
        &serial,
        &componentlenght,
        &flags,
        FileSystemNameBuffer,
        MAX_SIZE
    );
    
    if(info){
        cout<<"Volume name: "<<VolumeNameBuffer<<endl;
        cout<<"Serial number: "<<serial<<endl;
        cout<<"File system: "<<FileSystemNameBuffer<<endl;
        cout<<"Max length of file name component: "<<componentlenght<<endl;
        cout<<"File system flags: "<<flags<<endl;

        if(flags & FILE_CASE_SENSITIVE_SEARCH) cout<<"  - Supports case-sensitive file names"<<endl;
        if(flags & FILE_CASE_PRESERVED_NAMES) cout<<"  - Preserves case of file names"<<endl;
        if(flags & FILE_UNICODE_ON_DISK) cout<<"  - Supports Unicode in file names"<<endl;
        if(flags & FILE_PERSISTENT_ACLS) cout<<"  - Preserves and enforces ACLs"<<endl;
        if(flags & FILE_FILE_COMPRESSION) cout<<"  - Supports file-based compression"<<endl;
        if(flags & FILE_VOLUME_QUOTAS) cout<<"  - Supports disk quotas"<<endl;
        if(flags & FILE_SUPPORTS_SPARSE_FILES) cout<<"  - Supports sparse files"<<endl;
        if(flags & FILE_SUPPORTS_REPARSE_POINTS) cout<<"  - Supports reparse points"<<endl;
        if(flags & FILE_SUPPORTS_REMOTE_STORAGE) cout<<"  - Supports remote storage"<<endl;
        if(flags & FILE_VOLUME_IS_COMPRESSED) cout<<"  - Volume is compressed"<<endl;
        if(flags & FILE_SUPPORTS_OBJECT_IDS) cout<<"  - Supports object identifiers"<<endl;
        if(flags & FILE_SUPPORTS_ENCRYPTION) cout<<"  - Supports Encrypted File System (EFS)"<<endl;
        if(flags & FILE_NAMED_STREAMS) cout<<"  - Supports named streams"<<endl;
        if(flags & FILE_READ_ONLY_VOLUME) cout<<"  - Volume is read-only"<<endl;
        if(flags & FILE_SUPPORTS_TRANSACTIONS) cout<<"  - Supports transactions"<<endl;
        if(flags & FILE_SUPPORTS_HARD_LINKS) cout<<"  - Supports hard links"<<endl;
        if(flags & FILE_SUPPORTS_EXTENDED_ATTRIBUTES) cout<<"  - Supports extended attributes"<<endl;
    }
    else{
        cout<<"Error getting volume information! Error code: "<<GetLastError()<<endl;
    }
}

void showDiskFreeSpace(const vector<string>& listofdrivers){
    if(listofdrivers.empty()){
        cout<<"Please check your drives first (Option 1-2 in menu)"<<endl; 
        return;
    }
    
    DWORD SectorsPerCluster, BytesPerSector, NumberOfFreeClusters, TotalNumberOfClusters;
    
    int id;
    cout<<"Please select a disk number: ";
    cin >> id;
    
    if(id < 0 || id >= (int)listofdrivers.size()){
        cout<<"Invalid disk number!"<<endl;
        return;
    }
    
    BOOL space = GetDiskFreeSpaceA(
        listofdrivers[id].c_str(),
        &SectorsPerCluster,
        &BytesPerSector,
        &NumberOfFreeClusters,
        &TotalNumberOfClusters
    );
    
    if(space){
        __int64 totalBytes = (__int64)TotalNumberOfClusters * SectorsPerCluster * BytesPerSector;
        __int64 freeBytes = (__int64)NumberOfFreeClusters * SectorsPerCluster * BytesPerSector;
        
        cout<<"Sectors per cluster: "<<SectorsPerCluster<<endl;
        cout<<"Bytes per sector: "<<BytesPerSector<<endl;
        cout<<"Number of free clusters: "<<NumberOfFreeClusters<<endl;
        cout<<"Total clusters: "<<TotalNumberOfClusters<<endl;
        cout<<"\nTotal disk space: "<<totalBytes / (1024 * 1024)<<" MB ("<<totalBytes<<" bytes)"<<endl;
        cout<<"Free disk space: "<<freeBytes / (1024 * 1024)<<" MB ("<<freeBytes<<" bytes)"<<endl;
    }
    else{
        cout<<"Error getting disk free space! Error code: "<<GetLastError()<<endl;
    }
}

void ShowFileAttributes(const string& filePath){
    DWORD attribute = GetFileAttributesA(filePath.c_str());
    if(attribute == INVALID_FILE_ATTRIBUTES){
        cout<<"Error: Could not get file attributes! Error code: "<<GetLastError()<<endl;
        return;
    }
    
    cout<<"Attributes of file \""<<filePath<<"\":"<<endl;
    cout<<"----------------------------------------"<<endl;
    
    bool hasAttributes = false;
    
    if(attribute & FILE_ATTRIBUTE_DIRECTORY){
        cout<<"  - Directory"<<endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_READONLY){
        cout<<"  - Read-only"<<endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_HIDDEN){
        cout<<"  - Hidden"<<endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_SYSTEM){
        cout<<"  - System"<<endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_ARCHIVE){
        cout<<"  - Archive"<<endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_TEMPORARY){
        cout<<"  - Temporary"<<endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_NORMAL){
        cout<<"  - Normal file"<<endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_COMPRESSED){
        cout<<"  - Compressed"<<endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_OFFLINE){
        cout<<"  - Offline"<<endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED){
        cout<<"  - Not content indexed"<<endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_ENCRYPTED){
        cout<<"  - Encrypted"<<endl;
        hasAttributes = true;
    }
    
    if(!hasAttributes){
        cout<<"  - No special attributes"<<endl;
    }
    
    cout<<"----------------------------------------"<<endl;
    cout<<"Attribute value (hex): 0x"<<hex<<attribute<<dec<<endl;
}

void SetFileAttributesMenu(const string& filePath){
    int indexAttribute;
    bool bAttribute;
    DWORD newAttribute;
    
    cout<<"\n=== Set File Attributes ==="<<endl;
    cout<<"Current attributes:"<<endl;
    ShowFileAttributes(filePath);
    
    cout<<"\n=== Choose attribute to set ==="<<endl;
    cout<<"1.  ARCHIVE"<<endl;
    cout<<"2.  HIDDEN"<<endl;
    cout<<"3.  NORMAL (clears all other attributes)"<<endl;
    cout<<"4.  NOT CONTENT INDEXED"<<endl;
    cout<<"5.  OFFLINE"<<endl;
    cout<<"6.  READONLY"<<endl;
    cout<<"7.  SYSTEM"<<endl;
    cout<<"8.  TEMPORARY"<<endl;
    cout<<"9.  COMPRESSED"<<endl;
    cout<<"10. ENCRYPTED"<<endl;
    cout<<"0.  Cancel"<<endl;
    cout<<"Choose attribute: ";
    cin >> indexAttribute;
    
    switch(indexAttribute){
        case 1:
            newAttribute = FILE_ATTRIBUTE_ARCHIVE;
            break;
        case 2:
            newAttribute = FILE_ATTRIBUTE_HIDDEN;
            break;
        case 3:
            newAttribute = FILE_ATTRIBUTE_NORMAL;
            break;
        case 4:
            newAttribute = FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
            break;
        case 5:
            newAttribute = FILE_ATTRIBUTE_OFFLINE;
            break;
        case 6:
            newAttribute = FILE_ATTRIBUTE_READONLY;
            break;
        case 7:
            newAttribute = FILE_ATTRIBUTE_SYSTEM;
            break;
        case 8:
            newAttribute = FILE_ATTRIBUTE_TEMPORARY;
            break;
        case 9:
            newAttribute = FILE_ATTRIBUTE_COMPRESSED;
            break;
        case 10:
            newAttribute = FILE_ATTRIBUTE_ENCRYPTED;
            break;
        case 0:
            cout<<"Operation cancelled."<<endl;
            return;
        default:
            cout<<"Invalid choice!"<<endl;
            return;
    }
    
    bAttribute = SetFileAttributesA(filePath.c_str(), newAttribute);
    if(bAttribute){
        cout<<"Attributes successfully changed!"<<endl;
        cout<<"New attributes:"<<endl;
        ShowFileAttributes(filePath);
    }
    else{
        cout<<"Error setting attributes! Error code: "<<GetLastError()<<endl;
        if(GetLastError() == ERROR_ACCESS_DENIED){
            cout<<"Access denied. Try running as administrator."<<endl;
        }
    }
}

void GetFileInformation(const string& filePath){
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
        cout<<"Error opening file! Error code: "<<GetLastError()<<endl;
        return;
    }
    
    if(GetFileInformationByHandle(hInfFile, &lpFileInformation)){
        cout<<"\n=== File Information ==="<<endl;
        cout<<"File: "<<filePath<<endl;
        cout<<"----------------------------------------"<<endl;
        
        // Attributes
        cout<<"Attributes: ";
        if(lpFileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) cout<<"Directory ";
        if(lpFileInformation.dwFileAttributes & FILE_ATTRIBUTE_READONLY) cout<<"Read-only ";
        if(lpFileInformation.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) cout<<"Hidden ";
        if(lpFileInformation.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) cout<<"System ";
        if(lpFileInformation.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) cout<<"Archive ";
        if(lpFileInformation.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) cout<<"Temporary ";
        cout<<endl;
        
        // Time information
        printFileTimeForHandle(lpFileInformation.ftCreationTime, "Created");
        printFileTimeForHandle(lpFileInformation.ftLastAccessTime, "Last access");
        printFileTimeForHandle(lpFileInformation.ftLastWriteTime, "Last modified");
        
        // Size information
        fileSize = (static_cast<ULONGLONG>(lpFileInformation.nFileSizeHigh)<<32) | lpFileInformation.nFileSizeLow;
        cout<<"\nSize: "<<fileSize<<" bytes";
        if(fileSize > 1024) cout<<" ("<<fixed<<setprecision(2)<<(double)fileSize / 1024<<" KB)";
        if(fileSize > 1024 * 1024) cout<<" ("<<fixed<<setprecision(2)<<(double)fileSize / (1024 * 1024)<<" MB)";
        cout<<endl;
        
        // Volume information
        cout<<"Volume serial number: 0x"<<hex<<lpFileInformation.dwVolumeSerialNumber<<dec<<endl;
        
        // Link information
        cout<<"Number of hard links: "<<lpFileInformation.nNumberOfLinks<<endl;
        if(lpFileInformation.nNumberOfLinks > 1){
            cout<<"  (This file has multiple hard links)"<<endl;
        }
        
        // File ID
        fileId = (static_cast<ULONGLONG>(lpFileInformation.nFileIndexHigh)<<32) | lpFileInformation.nFileIndexLow;
        cout<<"Unique file ID: 0x"<<hex<<fileId<<dec<<endl;
        
        cout<<"----------------------------------------"<<endl;
    }
    else{
        cout<<"Error getting file information! Error code: "<<GetLastError()<<endl;
    }
    
    CloseHandle(hInfFile);
}

void GetFileTimes(const string& filePath){
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
        cout<<"Error opening file! Error code: "<<GetLastError()<<endl;
        return;
    }
    
    if(GetFileTime(hFile, &createTime, &accessTime, &writeTime)){
        cout<<"\n=== File Time Information ==="<<endl;
        cout<<"File: "<<filePath<<endl;
        cout<<"----------------------------------------"<<endl;
       printFileTimeForTime(createTime, "Created");
       printFileTimeForTime(accessTime, "Last accessed");
       printFileTimeForTime(writeTime, "Last modified");
        cout<<"----------------------------------------"<<endl;
    }
    else{
        cout<<"Failed to get file time! Error code: "<<GetLastError()<<endl;
    }
    
    CloseHandle(hFile);
}

void SetFileTimes(const string& filePath){
    int TimeID, year, month, day, hour, minute, second;
    DWORD flags, attrs;
    HANDLE hFile;
    FILETIME verifyCreate, verifyAccess, verifyWrite;
    
    attrs = GetFileAttributesA(filePath.c_str());
    if(attrs == INVALID_FILE_ATTRIBUTES){
        cout<<"Error: File does not exist! Error code: "<<GetLastError()<<endl;
        return;
    }
    
    cout<<"\n=== Current File Times ==="<<endl;
    GetFileTimes(filePath);
    
    cout<<"\n=== Select time mode ==="<<endl;
    cout<<"1. Set to current system time"<<endl;
    cout<<"2. Set custom date and time"<<endl;
    cout<<"3. Set creation time only"<<endl;
    cout<<"4. Set last access time only"<<endl;
    cout<<"5. Set last modified time only"<<endl;
    cout<<"Choose (1-5): ";
    cin >> TimeID;
    
    flags = FILE_ATTRIBUTE_NORMAL;
    if(attrs & FILE_ATTRIBUTE_DIRECTORY){
        flags = FILE_FLAG_BACKUP_SEMANTICS;
        cout<<"Note: Target is a directory"<<endl;
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
        cout<<"Error opening file! Error code: "<<GetLastError()<<endl;
        return;
    }
    
    SYSTEMTIME st = {0};
    FILETIME newTime, newCreateTime, newAccessTime, newWriteTime;
    BOOL success = FALSE;
    
    switch (TimeID){
        case 1: 
        {
            GetSystemTime(&st);
            SystemTimeToFileTime(&st, &newTime);
            cout<<"Setting all file times to current system time..."<<endl;
            success = SetFileTime(hFile, &newTime, &newTime, &newTime);
        }
        break;
        
        case 2: 
        {
            cout<<"\nEnter custom date and time:"<<endl;
            cout<<"Year (1601-30827): ";
            cin >> year;
            cout<<"Month (1-12): ";
            cin >> month;
            cout<<"Day (1-31): ";
            cin >> day;
            cout<<"Hour (0-23): ";
            cin >> hour;
            cout<<"Minute (0-59): ";
            cin >> minute;
            cout<<"Second (0-59): ";
            cin >> second;
            
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
                cout<<"Error: Invalid date or time!"<<endl;
                CloseHandle(hFile);
                return;
            }
            
            if(!SystemTimeToFileTime(&st, &newTime)){
                cout<<"Error converting time! Error code: "<<GetLastError()<<endl;
                CloseHandle(hFile);
                return;
            }
            
            cout<<"Setting all file times to custom date/time..."<<endl;
            success = SetFileTime(hFile, &newTime, &newTime, &newTime);
        }
        break;
        
        case 3: 
        {
            cout<<"\nEnter custom creation date and time:"<<endl;
            cout<<"Year (1601-30827): ";
            cin >> year;
            cout<<"Month (1-12): ";
            cin >> month;
            cout<<"Day (1-31): ";
            cin >> day;
            cout<<"Hour (0-23): ";
            cin >> hour;
            cout<<"Minute (0-59): ";
            cin >> minute;
            cout<<"Second (0-59): ";
            cin >> second;
            
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
                cout<<"Error: Invalid date or time!"<<endl;
                CloseHandle(hFile);
                return;
            }
            
            if(!SystemTimeToFileTime(&st, &newCreateTime)){
                cout<<"Error converting time! Error code: "<<GetLastError()<<endl;
                CloseHandle(hFile);
                return;
            }
            
            cout<<"Setting creation time only..."<<endl;
            success = SetFileTime(hFile, &newCreateTime, NULL, NULL);
        }
        break;
        
        case 4: 
        {
            cout<<"\nEnter custom last access date and time:"<<endl;
            cout<<"Year (1601-30827): ";
            cin >> year;
            cout<<"Month (1-12): ";
            cin >> month;
            cout<<"Day (1-31): ";
            cin >> day;
            cout<<"Hour (0-23): ";
            cin >> hour;
            cout<<"Minute (0-59): ";
            cin >> minute;
            cout<<"Second (0-59): ";
            cin >> second;
            
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
                cout<<"Error: Invalid date or time!"<<endl;
                CloseHandle(hFile);
                return;
            }
            
            if(!SystemTimeToFileTime(&st, &newAccessTime)){
                cout<<"Error converting time! Error code: "<<GetLastError()<<endl;
                CloseHandle(hFile);
                return;
            }
            
            cout<<"Setting last access time only..."<<endl;
            success = SetFileTime(hFile, NULL, &newAccessTime, NULL);
        }
        break;
        
        case 5: 
        {
            cout<<"\nEnter custom last modified date and time:"<<endl;
            cout<<"Year (1601-30827): ";
            cin >> year;
            cout<<"Month (1-12): ";
            cin >> month;
            cout<<"Day (1-31): ";
            cin >> day;
            cout<<"Hour (0-23): ";
            cin >> hour;
            cout<<"Minute (0-59): ";
            cin >> minute;
            cout<<"Second (0-59): ";
            cin >> second;
            
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
                cout<<"Error: Invalid date or time!"<<endl;
                CloseHandle(hFile);
                return;
            }
            
            if(!SystemTimeToFileTime(&st, &newWriteTime)){
                cout<<"Error converting time! Error code: "<<GetLastError()<<endl;
                CloseHandle(hFile);
                return;
            }
            
            cout<<"Setting last modified time only..."<<endl;
            success = SetFileTime(hFile, NULL, NULL, &newWriteTime);
        }
        break;
        
        default:
            cout<<"Error: Invalid choice (1-5)!"<<endl;
            CloseHandle(hFile);
            return;
    }
    
    if(success){
        cout<<"\nFile times successfully updated!"<<endl;
        cout<<"\n=== New File Times ==="<<endl;
        
        if(GetFileTime(hFile, &verifyCreate, &verifyAccess, &verifyWrite)){
           printFileTimeForTime(verifyCreate, "Created");
           printFileTimeForTime(verifyAccess, "Last accessed");
           printFileTimeForTime(verifyWrite, "Last modified");
        }
    }else{
        cout<<"Error setting file times! Error code: "<<GetLastError()<<endl;
        if(GetLastError() == ERROR_ACCESS_DENIED){
            cout<<"Access denied. Try running as administrator."<<endl;
        }
    }
    CloseHandle(hFile);
}

int main()
{   
    int flag = 1; 
    int menu;
    vector<string> listofdrivers;
    
    while(flag){
        cout<<"\n=====Menu=====\n";
        cout<<"0.  Exit"<<endl;
        cout<<"1.  Output list of disks (GetLogicalDriveStrings)"<<endl;
        cout<<"2.  Output list of disks (GetLogicalDrives)"<<endl;
        cout<<"3.  Driver Type (GetDriverType)"<<endl;
        cout<<"4.  Driver info (GetVolumeInformation)"<<endl;
        cout<<"5.  Driver space (GetDiskFreeSpace)"<<endl;
        cout<<"6.  Create file in directory"<<endl;
        cout<<"7.  Copy file in directory"<<endl;
        cout<<"8.  Move file in directory"<<endl;
        cout<<"9.  Create directory"<<endl;
        cout<<"10. Remove directory"<<endl;
        cout<<"11. Move file (Extended)"<<endl;
        cout<<"12. File Attribute (GetFileAttributes)"<<endl;
        cout<<"13. Set Attribute (SetFileAttributes)"<<endl;
        cout<<"14. Get file info (GetFileInformationByHandle)"<<endl;
        cout<<"15. Get file time (GetFileTime)"<<endl;
        cout<<"16. Set file time (SetFileTime)"<<endl;
        cout<<"Choose function: ";
        cin>>menu;
        cout<<"\n";
        
        switch(menu){
            case 1:  
                showDrivesByGetLogicalDrivestrings(listofdrivers);
                break;
            case 2:
                showDrivesByGetLogicalDrives(listofdrivers);
                break;
            case 3:
                showDriveType(listofdrivers);
                break;
            case 4:
                showVolumeInformation(listofdrivers);
                break;
            case 5:
                showDiskFreeSpace(listofdrivers);
                break;
            case 6:
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
            case 7:
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
            case 8:
            {
                string sourcePath;
                string destPath;
                cin.ignore();
                cout<<"Enter source file path: ";
                getline(cin, sourcePath);
                cout<<"Enter destination (file or directory): ";
                getline(cin, destPath);
                moveFile(sourcePath, destPath);
                break;
            }
            case 9: 
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
            case 10:
            {
                string dirPath;
                cin.ignore();
                cout<<"Enter directory Path: ";
                getline(cin, dirPath);
                removeDirectory(dirPath);
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
                cout<<"Enter destination (file or directory): ";
                getline(cin, destPath);
                
                cout<<"\nSelect MoveFileEx flags:"<<endl;
                cout<<"1. Normal move (no special flags)"<<endl;
                cout<<"2. Replace existing file"<<endl;
                cout<<"3. Allow copy between different drives"<<endl;
                cout<<"4. Replace + Allow copy"<<endl;
                cout<<"Choose flag option: ";
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