#include "fileManager.hpp"

void removeDirectory(const string& dirPath){
    string path =trimmed(dirPath);
    
    cout<<"Removing directory: "<<path<<endl;
    
    bool isDir;
    if(!checkPath(path, isDir)){
            cout<<"Error: Directory does not exist!"<<endl;
            return;
        }
    
    if(!isDir){
            cout<<"Error: Path is not a directory!"<<endl;
            return;
        }
        
    if(checkContain(path)){
        cout<<"Error: Directory is not empty! Please delete all files first."<<endl;
        return;
        }
        
    if(RemoveDirectory(path.c_str())){
            cout<<"Directory successfully removed: "<<path<<endl;
            return;
    }else{
        DWORD error = GetLastError();
        if(error == ERROR_ACCESS_DENIED){
                cout<<"Error: Access denied!"<<endl;
        }else{
            cout<<"Error: failed to remove directory! Error code: "<<error<<endl;
        }
        return;
    }
}

void createFile(const string& dirPath, const string& fileName, const string& content){
    string fullPath = trimmed(dirPath)+ "\\" + fileName;
    
    cout<<"Creating file: "<<fullPath<<endl;
    
    HANDLE hFile = CreateFile(
        fullPath.c_str(),        //путь к файлу
        GENERIC_WRITE,           //доступ: запись
        0,                       //режим совместного доступа: исключительный
        NULL,                    //атрибуты безопасности: стандартные
        CREATE_ALWAYS,           //создаем новый/перезаписываем существующий
        FILE_ATTRIBUTE_NORMAL,   //обычный файл (без атрибутов)
        NULL                     //шаблон файла: не используется
    );
    
    if(hFile == INVALID_HANDLE_VALUE){
        cout<<"Error: failed to create file!"<<endl;
        return;
    }
    
    if(!content.empty()){
        DWORD bytesWritten;
        if(!WriteFile(hFile, content.c_str(), content.length(), &bytesWritten, NULL)){
            cout<<"Error: failed to write to file!"<<endl;
            CloseHandle(hFile);
            return;
        }
        cout<<"Written "<<bytesWritten<<" bytes to file"<<endl;
    }
    //Закрываем дескриптор файла (освобождаем ресурс)
    CloseHandle(hFile);
    cout<<"File successfully created: "<<fullPath<<endl;
    return;
}

void copyFile(const string& sourcePath, const string& destDir){
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
            return;
        }
    }
    
    if(CopyFile(trimmed(sourcePath).c_str(), destPath.c_str(), FALSE)){
        cout<<"File successfully copied!"<<endl;
        return;
    }else{
        cout<<"Error: failed to copy file!"<<endl;
        return;
    }
}

void moveFile(const string& sourcePath, const string& destPath){
    string src = trimmed(sourcePath);
    string dst = trimmed(destPath);
    string fileName = getFileName(src);

    bool isDir;
    bool destExists;
    bool destIsDir;

    if(checkPath(dst, isDir) && isDir){
        dst = dst + "\\" + fileName;
    }

    destExists = checkPath(dst, destIsDir);

    size_t pos = dst.find_last_of("\\/");
    string destDir = (pos == string::npos) ? "" : dst.substr(0, pos);

    bool dirExists = true;
    if(!destDir.empty()){
        if(!checkPath(destDir, dirExists) || !dirExists){
            cout << "Error: destination directory does not exist!" << endl;
            return;
        }
    }

    if(destExists && !destIsDir){
        cout<<"Destination file already exists!"<<endl;
        cout<<"Overwrite? (y/n): ";
        char choice;
        cin>>choice;

        if(tolower(choice) != 'y'){
            cout<<"Move cancelled."<<endl;
            return;
        }

        if(!DeleteFile(dst.c_str())){
            cout<<"Error: failed to delete existing file!"<<endl;
            return;
        }
    }
    else if(destExists && destIsDir){
        cout<<"Error: destination is a directory! Please specify a file name."<<endl;
        return;
    }

    if(MoveFile(src.c_str(), dst.c_str())){
        cout<<"File successfully moved!"<<endl;
        cout<<"From: "<<src<<endl;
        cout<<"To: "<<dst<<endl;
        return;
    }else{
        DWORD error = GetLastError();

        if(error == ERROR_ACCESS_DENIED){
            cout<<"Error: Access denied!"<<endl;
        }
        else if(error == ERROR_NOT_SAME_DEVICE){
            cout<<"Error: Cannot move between different drives!"<<endl;
        }
        else{
            cout<<"Error: failed to move file! Error code: "<<error<<endl;
        }

        return;
    }
}

void moveFileEx(const string& srcPath, const string& dstPath, DWORD flags){
    BOOL result = MoveFileExA(srcPath.c_str(), dstPath.c_str(), flags);

    if (result)
    {
        cout << "Move successful\n";
        cout << "From: " << srcPath << endl;
        cout << "To:   " << dstPath << endl;
        return;
    }
    else
    {
        DWORD error = GetLastError();
        cout << "Move failed. Error code: " << error << endl;
        switch (error)
        {
            case ERROR_ACCESS_DENIED:
                cout << "Access denied. Check if:\n";
                cout << "  - Destination file is not read-only\n";
                cout << "  - Destination file is not open in another program\n";
                cout << "  - You have write permissions\n";
                break;
            case ERROR_PATH_NOT_FOUND:cout << "Path not found. Check if destination directory exists\n";break;
            case ERROR_FILE_NOT_FOUND:cout << "Source file not found\n";break;
            case ERROR_ALREADY_EXISTS:cout << "Destination file already exists. Use REPLACE_EXISTING flag\n";break;
            case ERROR_NOT_SAME_DEVICE:cout << "Different drives. Use MOVEFILE_COPY_ALLOWED flag\n";break;
            case ERROR_INVALID_PARAMETER:cout << "Invalid parameter. Check paths and flags\n";break;
            default:cout << "Unhandled error\n";break;
        }

        return;
    }
}

void showFileAttributes(const string& filePath){
    DWORD attribute = GetFileAttributesA(filePath.c_str());
    if(attribute == INVALID_FILE_ATTRIBUTES){
        cout<<"Error: Could not get file attributes! Error code: "<<GetLastError()<<endl;
        return;
    }
    
    cout<<"Attributes of file \""<<filePath<<"\":"<<endl;
    
    bool hasAttributes = false;
    
    if(attribute & FILE_ATTRIBUTE_DIRECTORY){
        cout<<" Directory"<<endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_READONLY){
        cout<<" Read-only"<<endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_HIDDEN){
        cout<<" Hidden"<<endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_SYSTEM){
        cout<<" System"<<endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_ARCHIVE){
        cout<<" Archive"<<endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_TEMPORARY){
        cout<<" Temporary"<<endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_NORMAL){
        cout<<" Normal file"<<endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_COMPRESSED){
        cout<<" Compressed"<<endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED){
        cout<<" Not content indexed"<<endl;
        hasAttributes = true;
    }
    if(attribute & FILE_ATTRIBUTE_ENCRYPTED){
        cout<<" Encrypted"<<endl;
        hasAttributes = true;
    }
    
    if(!hasAttributes){
        cout<<"  - No special attributes"<<endl;
    }
    cout<<"Attribute value (hex): 0x"<<hex<<attribute<<dec<<endl;
}

void setFileAttribute(const string& filePath){
    int indexAttribute;
    bool bAttribute;
    DWORD newAttribute;
    
    cout<<"Current attributes:"<<endl;
    showFileAttributes(filePath);
    
    cout<<"\nChoose attribute to set:"<<endl;
    cout<<"(1)  ARCHIVE"<<endl;
    cout<<"(2)  HIDDEN"<<endl;
    cout<<"(3)  NORMAL (clears all other attributes)"<<endl;
    cout<<"(4)  NOT CONTENT INDEXED"<<endl;
    cout<<"(5)  ENCRYPTED"<<endl;
    cout<<"(6)  READONLY"<<endl;
    cout<<"(7)  SYSTEM"<<endl;
    cout<<"(8)  TEMPORARY"<<endl;
    cout<<"(9)  COMPRESSED"<<endl;
    cout<<"Choose attribute: ";
    cin >> indexAttribute;
    
    switch(indexAttribute){
        case 1: newAttribute = FILE_ATTRIBUTE_ARCHIVE;break;
        case 2: newAttribute = FILE_ATTRIBUTE_HIDDEN;break;
        case 3: newAttribute = FILE_ATTRIBUTE_NORMAL;break;
        case 4: newAttribute = FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;break;
        case 5: newAttribute = FILE_ATTRIBUTE_ENCRYPTED;break;
        case 6: newAttribute = FILE_ATTRIBUTE_READONLY;break;
        case 7: newAttribute = FILE_ATTRIBUTE_SYSTEM;break;
        case 8: newAttribute = FILE_ATTRIBUTE_TEMPORARY;break;
        case 9: newAttribute = FILE_ATTRIBUTE_COMPRESSED;break;
        default: cout<<"Invalid choice!"<<endl;return;
    }
    
    bAttribute = SetFileAttributesA(filePath.c_str(), newAttribute);
    if(bAttribute){
        cout<<"Attributes successfully changed!"<<endl;
        cout<<"New attributes:"<<endl;
        showFileAttributes(filePath);
    }
    else{
        cout<<"Error setting attributes! Error code: "<<GetLastError()<<endl;
        if(GetLastError() == ERROR_ACCESS_DENIED){
            cout<<"Access denied. Try running as administrator."<<endl;
        }
    }
}

void showFileInfo(const string& filePath){
    ULONGLONG fileSize;
    ULONGLONG fileId;
     
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
        cout<<"\nFile Information:"<<endl;
        cout<<"File: "<<filePath<<endl;
        
        //Attributes
        cout<<"Attributes: ";
        if(lpFileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) cout<<"Directory ";
        if(lpFileInformation.dwFileAttributes & FILE_ATTRIBUTE_READONLY) cout<<"Read-only ";
        if(lpFileInformation.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) cout<<"Hidden ";
        if(lpFileInformation.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) cout<<"System ";
        if(lpFileInformation.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) cout<<"Archive ";
        if(lpFileInformation.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) cout<<"Temporary ";
        cout<<endl;
        
        //Time information
        printFileTimeForHandle(lpFileInformation.ftCreationTime, "Created");
        printFileTimeForHandle(lpFileInformation.ftLastAccessTime, "Last access");
        printFileTimeForHandle(lpFileInformation.ftLastWriteTime, "Last modified");
        
        //Size information
        fileSize = (static_cast<ULONGLONG>(lpFileInformation.nFileSizeHigh)<<32) | lpFileInformation.nFileSizeLow;
        cout<<"\nSize: "<<fileSize<<" bytes";
        if(fileSize > 1024) cout<<" ("<<fixed<<setprecision(2)<<(double)fileSize / 1024<<" KB)";
        if(fileSize > 1024 * 1024) cout<<" ("<<fixed<<setprecision(2)<<(double)fileSize / (1024 * 1024)<<" MB)";
        cout<<endl;
        
        //Volume information
        cout<<"Volume serial number: 0x"<<hex<<lpFileInformation.dwVolumeSerialNumber<<dec<<endl;
        
        //Link information
        cout<<"Number of hard links: "<<lpFileInformation.nNumberOfLinks<<endl;
        if(lpFileInformation.nNumberOfLinks > 1){
            cout<<"  (This file has multiple hard links)"<<endl;
        }
        
        //File ID
        fileId = (static_cast<ULONGLONG>(lpFileInformation.nFileIndexHigh)<<32) | lpFileInformation.nFileIndexLow;
        cout<<"Unique file ID: 0x"<<hex<<fileId<<dec<<endl;

    }
    else{
        cout<<"Error getting file information! Error code: "<<GetLastError()<<endl;
    }
    
    CloseHandle(hInfFile);
}

void showFileTimeInfo(const string& filePath){
    FILETIME createTime;
    FILETIME accessTime;
    FILETIME writeTime;
    
    HANDLE hFile = CreateFile(
        filePath.c_str(),    // Путь к файлу
        GENERIC_READ,        // Запрашиваем доступ только на чтение
        FILE_SHARE_READ,     // Разрешаем другим процессам читать файл
        NULL,                // Атрибуты безопасности (стандартные)
        OPEN_EXISTING,       // Открываем только существующий файл
        FILE_ATTRIBUTE_NORMAL, // Обычный файл
        NULL                 // Не используем шаблон файла
    );  
    
    if(hFile == INVALID_HANDLE_VALUE){
        cout<<"Error opening file! Error code: "<<GetLastError()<<endl;
        return;
    }
    
    if(GetFileTime(hFile, &createTime, &accessTime, &writeTime)){
        cout<<"\nFile Time Information:"<<endl;
        cout<<"File: "<<filePath<<endl;
        printFileTimeForTime(createTime, "Created");
        printFileTimeForTime(accessTime, "Last accessed");
        printFileTimeForTime(writeTime, "Last modified");
    }
    else{
        cout<<"Failed to get file time! Error code: "<<GetLastError()<<endl;
    }
    
    CloseHandle(hFile);
}

void setFileTime(const string& filePath){
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
    showFileTimeInfo(filePath);
    
    cout<<"\n=== Select time mode ==="<<endl;
    cout<<"(1) Set to current system time"<<endl;
    cout<<"(2) Set custom date and time"<<endl;
    cout<<"(3) Set creation time only"<<endl;
    cout<<"(4) Set last access time only"<<endl;
    cout<<"(5) Set last modified time only"<<endl;
    cout<<"Choose option: ";
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