#include "driversManager.hpp"

//Вывод дисков в системе
void showLogicalDrives(vector<string>& listofdrivers){
    int count = 1;
    listofdrivers.clear();
    DWORD drives = GetLogicalDrives();           //Получение битовой маски дисков 
    cout<<"Your drives: "<<endl;
    //Проверка битов и формирование пути для существующих дисков
    for(char d = 'A'; d <= 'Z'; d++){
        if(drives & 1){                          //Проверка младшего бита (текущий диск)
            string disk = string(1, d) + ":\\";
            cout<<count<<". "<<disk<<endl;
            listofdrivers.push_back(disk);
            count++;
        }
        drives >>= 1;                            //Сдвиг вправо для проверки следующего бита
    }
}

//Вывод списка дисков (через список строк)
void showLogicalDrivesSTR(vector<string>& listofdrivers){
    char lpBuffer[MAX_SIZE];                     //Буфер для приема строк от API
    int count = 1;
    listofdrivers.clear();
    
    DWORD result = GetLogicalDriveStringsA(MAX_SIZE, lpBuffer);  //Заполняет буфер: "C:\0D:\0\0". Возвращает кол-во символов или 0
    //Парсинг буфера, перебор строк до двойного null, и добавление пути в список listofdrivers
    if((result > 0) && (result <= MAX_SIZE)){
        cout<<"Your drives: "<<endl;
        char* drive = lpBuffer;                  //Указатель на начало буфера
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

//Вывод типа диска
void showDriveType(const vector<string>& listofdrivers){
    if(listofdrivers.empty()){
        cout<<"Please check your drives first (Option 1-2 in menu)"<<endl; 
        return;
    }
    
    int id;
    int number;
    cout<<"Please select a disk number: ";
    cin >> number;
    id = number - 1;
    
    if(id < 0 || id >= (int)listofdrivers.size()){
        cout<<"Invalid disk number!"<<endl;
        return;
    }
    
    UINT type = GetDriveTypeA(listofdrivers[id].c_str());
    switch(type){
        case 0:cout<<"DRIVE_UNKNOWN"<<endl;break;
        case 1:cout<<"DRIVE_NO_ROOT_DIR "<<endl;break;
        case 2:cout<<"DRIVE_REMOVABLE"<<endl;break;
        case 3:cout<<"DRIVE_FIXED"<<endl;break;
        case 4:cout<<"DRIVE_REMOTE"<<endl;break;
        case 5:cout<<"DRIVE_CDROM"<<endl;break;
        case 6:cout<<"DRIVE_RAMDISK"<<endl;break;
        default:cout<<"Unknown drive type: "<<type<<endl; break;
    }
}

//Вывод детальной  информации о томе
void showDriverInformation(const vector<string>& listofdrivers){
    if(listofdrivers.empty()){
        cout<<"Please check your drives first (Option 1-2 in menu)"<<endl; 
        return;
    }
    
    //Буферы для приема данных от API
    CHAR VolumeNameBuffer[MAX_SIZE];      //буфер для имени тома
    CHAR FileSystemNameBuffer[MAX_SIZE];  //буфер для файловой системы (NTFS, FAT32)
    DWORD componentlenght;                //максимальная длина компоненты имени файла
    DWORD flags;                          //флаги возможностей файловой системы
    DWORD serial;                         //серийный номер тома
    
    int id;
    cout<<"Please select a disk number: ";
    cin >> id;
    
    int index = id - 1;
    
    if(index < 0 || index >= (int)listofdrivers.size()){
        cout<<"Invalid disk number!"<<endl;
        return;
    }
    
    BOOL info = GetVolumeInformationA(
        listofdrivers[index].c_str(),   //путь к диску
        VolumeNameBuffer,               //буфер для имени тома
        MAX_SIZE,                       //размер буфера
        &serial,                        //указатель на серийный номер
        &componentlenght,               //указатель на максимальную длину имени
        &flags,                         //указатель на флаги
        FileSystemNameBuffer,           //буфер для файловой системы (NTFS, FAT32)
        MAX_SIZE                        //размер буфера
    );
    
    if(info){
        cout<<"Disk name: "<<VolumeNameBuffer<<endl;
        cout<<"Serial number: "<<serial<<endl;
        cout<<"File system: "<<FileSystemNameBuffer<<endl;
        cout<<"Max length of file name component: "<<componentlenght<<endl;
        cout<<"File system flags: "<<flags<<endl;

        if(flags & FILE_CASE_SENSITIVE_SEARCH)      //Файловая система чувствительна к регистру
            cout<<"  - Supports case-sensitive file names"<<endl;
        if(flags & FILE_CASE_PRESERVED_NAMES)       //Сохраняет регистр имен файлов
            cout<<"  - Preserves case of file names"<<endl;
        if(flags & FILE_UNICODE_ON_DISK)            //Поддерживает Unicode в именах
            cout<<"  - Supports Unicode in file names"<<endl;
        if(flags & FILE_PERSISTENT_ACLS)            //Поддерживает ACL (списки доступа)
            cout<<"  - Preserves and enforces ACLs"<<endl;
        if(flags & FILE_FILE_COMPRESSION)           //Поддерживает сжатие файлов
            cout<<"  - Supports file-based compression"<<endl;
        if(flags & FILE_VOLUME_QUOTAS)              //Поддерживает квоты диска
            cout<<"  - Supports disk quotas"<<endl;
        if(flags & FILE_SUPPORTS_SPARSE_FILES)      //Поддерживает разреженные файлы
            cout<<"  - Supports sparse files"<<endl;
        if(flags & FILE_SUPPORTS_REPARSE_POINTS)    //Поддерживает точки повторной обработки (символические ссылки)
            cout<<"  - Supports reparse points"<<endl;
        if(flags & FILE_SUPPORTS_REMOTE_STORAGE)    //Поддерживает удаленное хранение
            cout<<"  - Supports remote storage"<<endl;
        if(flags & FILE_VOLUME_IS_COMPRESSED)       //Весь том сжат
            cout<<"  - Volume is compressed"<<endl;
        if(flags & FILE_SUPPORTS_OBJECT_IDS)        //Поддерживает идентификаторы объектов
            cout<<"  - Supports object identifiers"<<endl;
        if(flags & FILE_SUPPORTS_ENCRYPTION)        //Поддерживает шифрование (EFS)
            cout<<"  - Supports Encrypted File System (EFS)"<<endl;
        if(flags & FILE_NAMED_STREAMS)              //Поддерживает именованные потоки (NTFS)
            cout<<"  - Supports named streams"<<endl;
        if(flags & FILE_READ_ONLY_VOLUME)           //Том только для чтения
            cout<<"  - Volume is read-only"<<endl;
        if(flags & FILE_SUPPORTS_TRANSACTIONS)      //Поддерживает транзакции (TxF)
            cout<<"  - Supports transactions"<<endl;
        if(flags & FILE_SUPPORTS_HARD_LINKS)        //Поддерживает жесткие ссылки
            cout<<"  - Supports hard links"<<endl;
        if(flags & FILE_SUPPORTS_EXTENDED_ATTRIBUTES) //Поддерживает расширенные атрибуты
            cout<<"  - Supports extended attributes"<<endl;
    }
    else{
        cout<<"Error getting volume information! Error code: "<<GetLastError()<<endl;
    }
}