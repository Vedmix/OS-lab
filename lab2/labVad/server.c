#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILENAME    "shared_file.dat"
#define MAPNAME     "Local\\SharedFileMap"
#define FILESIZE    4096            //file size in bytes
#define ACK_OFFSET (FILESIZE - 1) //acknowledgment byte offset

static HANDLE hFile    = INVALID_HANDLE_VALUE; //file handle
static HANDLE hMapping = NULL;                 //mapping handle
static char  *ptr      = NULL;                 //pointer to mapped memory


static void do_map(void)
{
    if(ptr != NULL){
        printf("[ERROR] File is already mapped.\n");
        return;
    }

    //create file on disk
    hFile = CreateFileA(
        FILENAME,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if(hFile == INVALID_HANDLE_VALUE){
        printf("[ERROR] CreateFile failed with error: %lu\n", GetLastError());
        return;
    }

    //fill file with zeros up to required size
    char buf[FILESIZE];
    memset(buf, 0, sizeof(buf));
    DWORD written;
    if(!WriteFile(hFile, buf, FILESIZE, &written, NULL)|| written != FILESIZE){
        printf("[ERROR] WriteFile failed with error: %lu\n", GetLastError());
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        return;
    }

    //create file mapping object
    hMapping = CreateFileMappingA(
        hFile,
        NULL,
        PAGE_READWRITE,
        0,
        FILESIZE,
        MAPNAME
    );
    if(hMapping == NULL){
        printf("[ERROR] CreateFileMapping failed with error: %lu\n", GetLastError());
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        return;
    }

    //map file into process address space
    ptr =(char *)MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, FILESIZE);
    if(ptr == NULL){
        printf("[ERROR] MapViewOfFile failed with error: %lu\n", GetLastError());
        CloseHandle(hMapping);
        CloseHandle(hFile);
        hMapping = NULL;
        hFile    = INVALID_HANDLE_VALUE;
        return;
    }

    printf("[INFO] File '%s' created and mapped into memory.\n", FILENAME);
    printf("       Mapping name: %s\n", MAPNAME);
    printf("       Mapping address: %p\n",(void *)ptr);
}

static void do_write(void)
{
    if(ptr == NULL){
        printf("[ERROR] Perform mapping first!\n");
        return;
    }

    char message[FILESIZE];
    printf("[Server] Enter string to write: ");
    if(fgets(message, sizeof(message), stdin)== NULL){
        printf("[ERROR] Input error!\n");
        return;
    }
    //remove newline character
    message[strcspn(message, "\n")] = '\0';

    //reset acknowledgment byte before writing
    ptr[ACK_OFFSET] = '\0';

    //write string to the beginning of mapped memory
    strncpy(ptr, message, FILESIZE - 2);
    ptr[FILESIZE - 2] = '\0';

    //flush changes to disk
    if(!FlushViewOfFile(ptr, FILESIZE))
        printf("[ERROR] FlushViewOfFile: %lu\n", GetLastError());

    printf("[INFO] Written: \"%s\"\n", ptr);
    printf("[INFO] Waiting for client...\n");

    //wait for client acknowledgment('\1' at the end of buffer)
    int waited = 0;
    while(ptr[ACK_OFFSET] != '\1'){
        Sleep(100); //100 ms

        waited++;
        if(waited % 20 == 0)
            printf("[INFO] Waiting for client confirmation...(%d s)\n", waited / 10);
        if(waited > 300){   //30 seconds
            printf("[INFO] Client wait timeout.\n");
            return;
        }
    }
    printf("[INFO] Client confirmed reading.\n");
}

static void do_exit(void)
{
    if(ptr != NULL){
        //unmap view
        UnmapViewOfFile(ptr);
        ptr = NULL;
        printf("[INFO] View unmapped.\n");
    }
    if(hMapping != NULL){
        CloseHandle(hMapping);
        hMapping = NULL;
    }
    if(hFile != INVALID_HANDLE_VALUE){
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

    //delete file from disk
    if(DeleteFileA(FILENAME))
        printf("[INFO] File '%s' deleted.\n", FILENAME);
    else if(GetLastError()!= ERROR_FILE_NOT_FOUND)
        printf("[ERROR] DeleteFile failed with error: %lu\n", GetLastError());

    printf("[INFO] Server terminated.\n");
    exit(EXIT_SUCCESS);
}

static void print_menu(void)
{
    printf("\n    Server menu:\n");
    printf("(1)Perform mapping\n");
    printf("(2)Write data\n");
    printf("(3)Exit\n");
    printf("Choose an option: ");
}

int main(void)
{
    char line[64];
    int  choice;

    printf("[INFO] Server started. PID = %lu\n", GetCurrentProcessId());

    for(;;){
        print_menu();
        if(fgets(line, sizeof(line), stdin)== NULL)
            break;
        choice = atoi(line);

        switch(choice){
        case 1: do_map();   break;
        case 2: do_write(); break;
        case 3: do_exit();  break;
        default:
            printf("[ERROR] Invalid menu option!\n");
        }
    }

    do_exit();
    return 0;
}