#include "driversManager.hpp"

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