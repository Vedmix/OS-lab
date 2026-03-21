#include <iostream>
#include <windows.h>
#include <vector>
#include <string.h>
#define MAX_SIZE 250

using namespace std;

string Trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

bool CheckPath(const string& path, bool& isDirectory) {
    DWORD attrs = GetFileAttributes(path.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    isDirectory = (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
    return true;
}

string GetCopyFileName(const string& originalPath) {
    string fileName = GetFileName(originalPath);
    size_t dotPos = fileName.find_last_of(".");
    
    if (dotPos != string::npos) {
        string nameWithoutExt = fileName.substr(0, dotPos);
        string ext = fileName.substr(dotPos);
        return nameWithoutExt + "_copy" + ext;
    } else {
        return fileName + "_copy";
    }
}

string GetFileName(const string& path) {
    size_t pos = path.find_last_of("\\");
    if (pos != string::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

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
        std::cout<<"4. Driver info (GetVolumeInformation)\n";
        std::cout<<"5. Driver space (GetDiskFreeSpace)\n";
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
            case 4:
            {
                if(listofdrivers.empty()){
                    std::cout<<"Please check your drives first (Option 1-2 in menu)"<<std::endl; 
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
                std::cout<<"Please select a disk number"<<std::endl;
                std::cin>>id;
                BOOL info = GetVolumeInformationA(listofdrivers[id].c_str(),lpVolumeNameBuffer,MAX_SIZE,lpVolumeSerialNumber,lpMaximumComponentLength,lpFileSystemFlags,lpFileSystemNameBuffer,MAX_SIZE);
                if(info){
                    std::cout<<"Tom: " << VolumeNameBuffer<< std::endl;
                    std::cout<<"Serial number: " << serial << std::endl;
                    std::cout<<"File system: "<< FileSystemNameBuffer<< std::endl;
                    std::cout<<"Max len of name: " << *lpMaximumComponentLength<< std::endl;
                    std::cout<<"File system flags: " << flags<<std::endl;
    
                    if(flags & FILE_CASE_SENSITIVE_SEARCH) std::cout<<"=The specified volume supports case-sensitive file names.="<<std::endl;
                    if(flags & FILE_CASE_PRESERVED_NAMES) std::cout<<"=The specified volume supports preserved case of file names when it places a name on disk.="<<std::endl;
                    if(flags & FILE_UNICODE_ON_DISK) std::cout<<"=The specified volume supports Unicode in file names as they appear on disk.="<<std::endl;
                    if(flags & FILE_PERSISTENT_ACLS) std::cout<<"=The specified volume preserves and enforces access control lists (ACL). For example, the NTFS file system preserves and enforces ACLs, and the FAT file system does not.="<<std::endl;
                    if(flags & FILE_FILE_COMPRESSION) std::cout<<"=The specified volume supports file-based compression.="<<std::endl;
                    if(flags & FILE_VOLUME_QUOTAS) std::cout<<"=The specified volume supports disk quotas.="<<std::endl;
                    if(flags & FILE_SUPPORTS_SPARSE_FILES) std::cout<<"=The specified volume supports sparse files.="<<std::endl;
                    if(flags & FILE_SUPPORTS_REPARSE_POINTS) std::cout<<"=The specified volume supports reparse points.="<<std::endl;
                    if(flags & FILE_SUPPORTS_REMOTE_STORAGE) std::cout<<"=The file system supports remote storage.="<<std::endl;
                    if(flags & FILE_RETURNS_CLEANUP_RESULT_INFO) std::cout<<"=On a successful cleanup operation, the file system returns information that describes additional actions taken during cleanup, such as deleting the file.="<<std::endl;
                    if(flags & FILE_SUPPORTS_POSIX_UNLINK_RENAME) std::cout<<"=The file system supports POSIX-style delete and rename operations.="<<std::endl;
                    if(flags & FILE_VOLUME_IS_COMPRESSED) std::cout<<"=The specified volume is a compressed volume, for example, a DoubleSpace volume.="<<std::endl;
                    if(flags & FILE_SUPPORTS_OBJECT_IDS) std::cout<<"=The specified volume supports object identifiers.="<<std::endl;
                    if(flags & FILE_SUPPORTS_ENCRYPTION) std::cout<<"=The specified volume supports the Encrypted File System (EFS).="<<std::endl;
                    if(flags & FILE_NAMED_STREAMS) std::cout<<"=The specified volume supports named streams.="<<std::endl;
                    if(flags & FILE_READ_ONLY_VOLUME) std::cout<<"=The specified volume is read-only.="<<std::endl;
                    if(flags & FILE_SEQUENTIAL_WRITE_ONCE) std::cout<<"=The specified volume supports a single sequential write.="<<std::endl;
                    if(flags & FILE_SUPPORTS_TRANSACTIONS) std::cout<<"=The specified volume supports transactions. For more information.="<<std::endl;
                    if(flags & FILE_SUPPORTS_HARD_LINKS) std::cout<<"=The specified volume supports hard links. For more information.="<<std::endl;
                    if(flags & FILE_SUPPORTS_EXTENDED_ATTRIBUTES) std::cout<<"=The specified volume supports extended attributes.="<<std::endl;
                    if(flags & FILE_SUPPORTS_OPEN_BY_FILE_ID) std::cout<<"=The file system supports open by FileID.="<<std::endl;
                    if(flags & FILE_SUPPORTS_USN_JOURNAL) std::cout<<"=The specified volume supports update sequence number (USN) journals.="<<std::endl;
                    if(flags & FILE_SUPPORTS_INTEGRITY_STREAMS) std::cout<<"=The file system supports integrity streams.="<<std::endl;
                    if(flags & FILE_SUPPORTS_BLOCK_REFCOUNTING) std::cout<<"=The specified volume supports sharing logical clusters between files on the same volume.="<<std::endl;
                    if(flags & FILE_SUPPORTS_SPARSE_VDL) std::cout<<"=The file system tracks whether each cluster of a file contains valid data.="<<std::endl;
                    if(flags & FILE_DAX_VOLUME) std::cout<<"=The specified volume is a direct access (DAX) volume.="<<std::endl;
                    if(flags & FILE_SUPPORTS_GHOSTING) std::cout<<"=The file system supports ghosting.="<<std::endl;

                }
                else{
                    std::cout<<"Error"<<std::endl;
                }

            }
            break;
            case 5:
            {
                if(listofdrivers.empty()){
                    std::cout<<"Please check your drives first (Option 1-2 in menu)"<<std::endl; 
                    break;
                }
                
                DWORD SectorsPerCluster, BytesPerSector, NumberOfFreeClusters,TotalNumberOfClusters;
                LPDWORD lpSectorsPerCluster = &SectorsPerCluster;
                LPDWORD lpBytesPerSector = &BytesPerSector;
                LPDWORD lpNumberOfFreeClusters = &NumberOfFreeClusters;
                LPDWORD lpTotalNumberOfClusters = &TotalNumberOfClusters;
                std::cout<<"Please select a disk number"<<std::endl;
                std::cin>>id;
                BOOL space = GetDiskFreeSpaceA(listofdrivers[id].c_str(),lpSectorsPerCluster,lpBytesPerSector,lpNumberOfFreeClusters,lpTotalNumberOfClusters);
                if(space){

                    std::cout<<"Sectors per clusters: " << SectorsPerCluster<<std::endl;
                    std::cout<<"Bytes per sector: " << BytesPerSector<<std::endl;
                    std::cout<<"Num of free clusters : " << NumberOfFreeClusters<<std::endl;
                    std::cout<<"Total clusters: " << TotalNumberOfClusters<<std::endl;
                }
                else{
                    std::cout<<"Error"<<std::endl;
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

    