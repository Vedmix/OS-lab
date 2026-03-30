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
        VolumeNameBuffer,               
        MAX_SIZE,                       //размер буфера
        &serial,                        
        &componentlenght,               
        &flags,                         
        FileSystemNameBuffer,           
        MAX_SIZE                        
    );
    
    if(info){
        cout<<"Disk name: "<<VolumeNameBuffer<<endl;
        cout<<"Serial number: "<<serial<<endl;
        cout<<"File system: "<<FileSystemNameBuffer<<endl;
        cout<<"Max length of file name component: "<<componentlenght<<endl;
        cout<<"File system info: "<<endl;

        if(flags & FILE_CASE_SENSITIVE_SEARCH) cout<<"  (+) Case-sensitive search"<<endl; //Файловая система чувствительна к регистру
        if(flags & FILE_CASE_PRESERVED_NAMES) cout<<"  (+) Preserves case of filenames"<<endl; //Сохраняет регистр имен файлов
        if(flags & FILE_UNICODE_ON_DISK) cout<<"  (+) Unicode filenames"<<endl; //Поддерживает Unicode в именах
        if(flags & FILE_FILE_COMPRESSION) cout<<"  (+) File-based compression"<<endl; //Поддерживает сжатие файлов
        if(flags & FILE_VOLUME_QUOTAS) cout<<"  (+) Disk quotas"<<endl; //Поддерживает квоты диска
        if(flags & FILE_SUPPORTS_SPARSE_FILES) cout<<"  (+) Sparse files"<<endl; //Поддерживает разреженные файлы
        if(flags & FILE_VOLUME_IS_COMPRESSED) cout<<"  (+) Volume is compressed"<<endl; //Весь том сжат
        if(flags & FILE_SUPPORTS_ENCRYPTION) cout<<"  (+) Encrypted File System (EFS)"<<endl; //Поддерживает шифрование (EFS)
        if(flags & FILE_NAMED_STREAMS) cout<<"  (+) NTFS"<<endl; //Поддерживает именованные потоки (NTFS)
        if(flags & FILE_READ_ONLY_VOLUME) cout<<"  (+) Volume is read-only"<<endl; //Том только для чтения
        if(flags & FILE_SUPPORTS_TRANSACTIONS) cout<<"  (+) Transactions"<<endl; //Поддерживает транзакции (TxF)
        if(flags & FILE_SUPPORTS_HARD_LINKS) cout<<"  (+) Hard links"<<endl; //Поддерживает жесткие ссылки
    }
    else{
        cout<<"Error getting volume information! Error code: "<<GetLastError()<<endl;
    }
}

//Получает информацию о свободном месте на диске
void showDriverFreeSpace(const vector<string>& listofdrivers){
    if(listofdrivers.empty()){
        cout<<"Please check your drives first (Option 1-2 in menu)"<<endl; 
        return;
    }
    
    DWORD SectorsPerCluster; // количество секторов в кластере
    DWORD BytesPerSector; // количество байт в секторе
    DWORD NumberOfFreeClusters; // количество свободных кластеров
    DWORD TotalNumberOfClusters; // общее количество кластеров
    
    int id;
    int number;
    cout<<"Please select a disk number: ";
    cin >> number;

    id = number - 1;
    
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
        
        cout<<"(1) Sectors per cluster: "<<SectorsPerCluster<<endl;
        cout<<"(2) Bytes per sector: "<<BytesPerSector<<endl;
        cout<<"(3) Number of free clusters: "<<NumberOfFreeClusters<<endl;
        cout<<"(4) Total clusters: "<<TotalNumberOfClusters<<endl;
        cout<<"(5) Total disk space: "<<totalBytes / (1024 * 1024)<<" MB"<<endl;
        cout<<"(6) Free disk space: "<<freeBytes / (1024 * 1024)<<" MB"<<endl;
    }
    else{
        cout<<"Error getting disk free space! Error code: "<<GetLastError()<<endl;
    }
}