#include <iostream>
#include <windows.h>
#include <vector>
#include <string.h>
#define MAX_SIZE 250

using namespace std;

string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

bool checkPath(const string& path, bool& isDirectory) {
    DWORD attrs = GetFileAttributes(path.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    isDirectory = (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
    return true;
}


string GetFileName(const string& path) {
    size_t pos = path.find_last_of("\\");
    if (pos != string::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

bool createFile(const string& dirPath, const string& fileName, const string& content = "") {
    string fullPath = trim(dirPath) + "\\" + fileName;
    
    cout << "Creating file: " << fullPath << endl;
    
    bool isDir;
    if (!checkPath(trim(dirPath), isDir) || !isDir) {
        cout << "Error: directory does not exist: " << trim(dirPath) << endl;
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
    
    if (hFile == INVALID_HANDLE_VALUE) {
        cout << "Error: failed to create file!" << endl;
        return false;
    }
    
    if (!content.empty()) {
        DWORD bytesWritten;
        if (!WriteFile(hFile, content.c_str(), content.length(), &bytesWritten, NULL)) {
            cout << "Error: failed to write to file!" << endl;
            CloseHandle(hFile);
            return false;
        }
        cout << "Written " << bytesWritten << " bytes to file" << endl;
    }
    
    CloseHandle(hFile);
    cout << "File successfully created: " << fullPath << endl;
    return true;
}


int main()
{   

    char lpBuffer[MAX_SIZE];
    int flag = 1; 
    char d;
    int menu, type, count, id;
    vector<string> listofdrivers;
    while(flag){
        cout<<"\n=====Menu=====\n";
        cout<<"1. Output list of disks (GetLogicalDriveStrings)\n";
        cout<<"2. Output list of disks (GetLogicalDrives)\n";
        cout<<"3. Driver Type (GetDriverType)\n";
        cout<<"4. Driver info (GetVolumeInformation)\n";
        cout<<"5. Driver space (GetDiskFreeSpace)\n";
        cout<<"6. Create file in directory"<<endl;
        cout<<"0. Exit\n\n";
        cin>>menu;
        cout<<"\n";
        count = 1;
        switch(menu){
            case 1:  
            {
                count = 1;
                listofdrivers.clear();
                DWORD result =  GetLogicalDriveStringsA(MAX_SIZE, lpBuffer); //string buffer (ANSI)
                if((result > 0) && (result<=MAX_SIZE)){
                    cout<<"Your drives: "<<endl;
                    char* drive = lpBuffer;
                    while(*drive){
                        cout<<count<<". "<<drive<<endl;
                        listofdrivers.push_back(string(drive));
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
                cout<<"Your drives: "<<endl;
                for(d = 'A';d <= 'Z'; d++){
                    if(drives & 1){
                        string disk = string(1,d) + ":\\";
                        cout<<count<<". "<<disk<<endl;
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
                    cout<<"Please check your drives first (Option 1-2 in menu)"<<endl; 
                    break;
                }
                cout<<"Please select a disk number"<<endl;
                cin>>id;
                UINT type = GetDriveTypeA(listofdrivers[id].c_str());
                switch(type){
                    case 0:{
                    cout<<"DRIVE_UNKNOWN (The drive type cannot be determined. )"<<endl;
                    }
                    break;
                    case 1:{
                    cout<<"DRIVE_NO_ROOT_DIR (The root path is invalid; for example, there is no volume mounted at the specified path. )"<<endl;
                    }
                    break;
                    case 2:{
                    cout<<"DRIVE_REMOVABLE (The drive has removable media; for example, a floppy drive, thumb drive, or flash card reader. )"<<endl;
                    }
                    break;
                    case 3:{
                    cout<<"DRIVE_FIXED (The drive has fixed media; for example, a hard disk drive or flash drive.)"<<endl;
                    }
                    break;
                    case 4:{
                    cout<<"DRIVE_CDROM (The drive is a remote (network) drive. )"<<endl;
                    }
                    break;
                    case 5:{
                    cout<<"DRIVE_CDROM (The drive is a CD-ROM drive. )"<<endl;
                    }
                    break;
                    case 6:{
                    cout<<"DRIVE_RAMDISK (The drive is a RAM disk. )"<<endl;
                    }
                    break;
                }
            }
            break;
            case 4:
            {
                if(listofdrivers.empty()){
                    cout<<"Please check your drives first (Option 1-2 in menu)"<<endl; 
                    break;
                }
                CHAR VolumeNameBuffer[MAX_SIZE];
                CHAR FileSystemNameBuffer[MAX_SIZE];
                DWORD componentlenght,flags;
                LPDWORD lpMaximumComponentLength = &componentlenght;
                DWORD serial;
                LPSTR lpVolumeNameBuffer = VolumeNameBuffer;
                LPDWORD lpVolumeSerialNumber = &serial;
                LPDWORD lpFileSystemFlags = &flags;
                LPSTR lpFileSystemNameBuffer = FileSystemNameBuffer;
                cout<<"Please select a disk number"<<endl;
                cin>>id;
                BOOL info = GetVolumeInformationA(listofdrivers[id].c_str(),lpVolumeNameBuffer,MAX_SIZE,lpVolumeSerialNumber,lpMaximumComponentLength,lpFileSystemFlags,lpFileSystemNameBuffer,MAX_SIZE);
                if(info){
                    cout<<"Tom: " << VolumeNameBuffer<< endl;
                    cout<<"Serial number: " << serial << endl;
                    cout<<"File system: "<< FileSystemNameBuffer<< endl;
                    cout<<"Max len of name: " << *lpMaximumComponentLength<< endl;
                    cout<<"File system flags: " << flags<<endl;
    
                    if(flags & FILE_CASE_SENSITIVE_SEARCH) cout<<"=The specified volume supports case-sensitive file names.="<<endl;
                    if(flags & FILE_CASE_PRESERVED_NAMES) cout<<"=The specified volume supports preserved case of file names when it places a name on disk.="<<endl;
                    if(flags & FILE_UNICODE_ON_DISK) cout<<"=The specified volume supports Unicode in file names as they appear on disk.="<<endl;
                    if(flags & FILE_PERSISTENT_ACLS) cout<<"=The specified volume preserves and enforces access control lists (ACL). For example, the NTFS file system preserves and enforces ACLs, and the FAT file system does not.="<<endl;
                    if(flags & FILE_FILE_COMPRESSION) cout<<"=The specified volume supports file-based compression.="<<endl;
                    if(flags & FILE_VOLUME_QUOTAS) cout<<"=The specified volume supports disk quotas.="<<endl;
                    if(flags & FILE_SUPPORTS_SPARSE_FILES) cout<<"=The specified volume supports sparse files.="<<endl;
                    if(flags & FILE_SUPPORTS_REPARSE_POINTS) cout<<"=The specified volume supports reparse points.="<<endl;
                    if(flags & FILE_SUPPORTS_REMOTE_STORAGE) cout<<"=The file system supports remote storage.="<<endl;
                    if(flags & FILE_RETURNS_CLEANUP_RESULT_INFO) cout<<"=On a successful cleanup operation, the file system returns information that describes additional actions taken during cleanup, such as deleting the file.="<<endl;
                    if(flags & FILE_SUPPORTS_POSIX_UNLINK_RENAME) cout<<"=The file system supports POSIX-style delete and rename operations.="<<endl;
                    if(flags & FILE_VOLUME_IS_COMPRESSED) cout<<"=The specified volume is a compressed volume, for example, a DoubleSpace volume.="<<endl;
                    if(flags & FILE_SUPPORTS_OBJECT_IDS) cout<<"=The specified volume supports object identifiers.="<<endl;
                    if(flags & FILE_SUPPORTS_ENCRYPTION) cout<<"=The specified volume supports the Encrypted File System (EFS).="<<endl;
                    if(flags & FILE_NAMED_STREAMS) cout<<"=The specified volume supports named streams.="<<endl;
                    if(flags & FILE_READ_ONLY_VOLUME) cout<<"=The specified volume is read-only.="<<endl;
                    if(flags & FILE_SEQUENTIAL_WRITE_ONCE) cout<<"=The specified volume supports a single sequential write.="<<endl;
                    if(flags & FILE_SUPPORTS_TRANSACTIONS) cout<<"=The specified volume supports transactions. For more information.="<<endl;
                    if(flags & FILE_SUPPORTS_HARD_LINKS) cout<<"=The specified volume supports hard links. For more information.="<<endl;
                    if(flags & FILE_SUPPORTS_EXTENDED_ATTRIBUTES) cout<<"=The specified volume supports extended attributes.="<<endl;
                    if(flags & FILE_SUPPORTS_OPEN_BY_FILE_ID) cout<<"=The file system supports open by FileID.="<<endl;
                    if(flags & FILE_SUPPORTS_USN_JOURNAL) cout<<"=The specified volume supports update sequence number (USN) journals.="<<endl;
                    if(flags & FILE_SUPPORTS_INTEGRITY_STREAMS) cout<<"=The file system supports integrity streams.="<<endl;
                    if(flags & FILE_SUPPORTS_BLOCK_REFCOUNTING) cout<<"=The specified volume supports sharing logical clusters between files on the same volume.="<<endl;
                    if(flags & FILE_SUPPORTS_SPARSE_VDL) cout<<"=The file system tracks whether each cluster of a file contains valid data.="<<endl;
                    if(flags & FILE_DAX_VOLUME) cout<<"=The specified volume is a direct access (DAX) volume.="<<endl;
                    if(flags & FILE_SUPPORTS_GHOSTING) cout<<"=The file system supports ghosting.="<<endl;

                }
                else{
                    cout<<"Error"<<endl;
                }

            }
            break;
            case 5:
            {
                if(listofdrivers.empty()){
                    cout<<"Please check your drives first (Option 1-2 in menu)"<<endl; 
                    break;
                }
                
                DWORD SectorsPerCluster, BytesPerSector, NumberOfFreeClusters,TotalNumberOfClusters;
                LPDWORD lpSectorsPerCluster = &SectorsPerCluster;
                LPDWORD lpBytesPerSector = &BytesPerSector;
                LPDWORD lpNumberOfFreeClusters = &NumberOfFreeClusters;
                LPDWORD lpTotalNumberOfClusters = &TotalNumberOfClusters;
                cout<<"Please select a disk number"<<endl;
                cin>>id;
                BOOL space = GetDiskFreeSpaceA(listofdrivers[id].c_str(),lpSectorsPerCluster,lpBytesPerSector,lpNumberOfFreeClusters,lpTotalNumberOfClusters);
                if(space){

                    cout<<"Sectors per clusters: " << SectorsPerCluster<<endl;
                    cout<<"Bytes per sector: " << BytesPerSector<<endl;
                    cout<<"Num of free clusters : " << NumberOfFreeClusters<<endl;
                    cout<<"Total clusters: " << TotalNumberOfClusters<<endl;
                }
                else{
                    cout<<"Error"<<endl;
                }

            }
            break;
            case 6: {
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
            }
            break;
            case 0:
            flag = 0;
            break;
        }
}
    return 0;
}

    