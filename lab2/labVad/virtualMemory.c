#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//состояние защиты в строку
static const char *protect_to_str(DWORD protect)
{
    switch (protect){
    case PAGE_NOACCESS:          return "PAGE_NOACCESS";
    case PAGE_READONLY:          return "PAGE_READONLY";
    case PAGE_READWRITE:         return "PAGE_READWRITE";
    case PAGE_WRITECOPY:         return "PAGE_WRITECOPY";
    case PAGE_EXECUTE:           return "PAGE_EXECUTE";
    case PAGE_EXECUTE_READ:      return "PAGE_EXECUTE_READ";
    case PAGE_EXECUTE_READWRITE: return "PAGE_EXECUTE_READWRITE";
    case PAGE_EXECUTE_WRITECOPY: return "PAGE_EXECUTE_WRITECOPY";
    case 0:                      return "N/A";
    default:                     return "UNKNOWN";
    }
}

//тип региона в строку
static const char *type_to_str(DWORD type)
{
    switch (type){
    case MEM_IMAGE:   return "MEM_IMAGE";
    case MEM_MAPPED:  return "MEM_MAPPED";
    case MEM_PRIVATE: return "MEM_PRIVATE";
    case 0:           return "N/A";
    default:          return "UNKNOWN";
    }
}

//состояние региона в строку
static const char *state_to_str(DWORD state)
{
    switch (state){
    case MEM_COMMIT:  return "MEM_COMMIT";
    case MEM_FREE:    return "MEM_FREE";
    case MEM_RESERVE: return "MEM_RESERVE";
    default:          return "UNKNOWN";
    }
}

static void do_system_info(void)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    printf("\n      System information:\n");
    printf("Processor architecture:       %u\n",   si.wProcessorArchitecture);
    printf("Page size:                    %lu bytes\n", si.dwPageSize);
    printf("Minimum application address:  %p\n",   si.lpMinimumApplicationAddress);
    printf("Maximum application address:  %p\n",   si.lpMaximumApplicationAddress);
    printf("Active processor mask:        0x%lX\n", si.dwActiveProcessorMask);
    printf("Number of processors:         %lu\n",   si.dwNumberOfProcessors);
    printf("Processor type:               %lu\n",   si.dwProcessorType);
    printf("Allocation granularity:       %lu bytes\n", si.dwAllocationGranularity);
    printf("Processor level:              %u\n",   si.wProcessorLevel);
    printf("Processor revision:           %u\n",   si.wProcessorRevision);
}

static void do_memory_status(void)
{
    MEMORYSTATUS ms;
    ms.dwLength=sizeof(ms);
    GlobalMemoryStatus(&ms);

    printf("\n      Memory status:\n");
    printf("Memory load:                  %lu%%\n",  ms.dwMemoryLoad);
    printf("Total physical memory:        %lu bytes\n", ms.dwTotalPhys);
    printf("Available physical memory:    %lu bytes\n", ms.dwAvailPhys);
    printf("Total page file:              %lu bytes\n", ms.dwTotalPageFile);
    printf("Available page file:          %lu bytes\n", ms.dwAvailPageFile);
    printf("Total virtual memory:         %lu bytes\n", ms.dwTotalVirtual);
    printf("Available virtual memory:     %lu bytes\n", ms.dwAvailVirtual);
}

static void do_virtual_query(void)
{
    char buf[64];
    printf("Enter address (in hexadecimal): ");
    if(fgets(buf, sizeof(buf), stdin)==NULL){
        printf("[ERROR] Input error!\n");
        return;
    }

    LPVOID addr=(LPVOID)(uintptr_t)strtoull(buf, NULL, 16);

    MEMORY_BASIC_INFORMATION mbi;
    SIZE_T result=VirtualQuery(addr, &mbi, sizeof(mbi));
    if(result==0){
        printf("[ERROR] VirtualQuery failed with error: %lu\n", GetLastError());
        return;
    }

    printf("\n      Memory region state at address %p:\n", addr);
    printf("Base address:                 %p\n",   mbi.BaseAddress);
    printf("Allocation base:              %p\n",   mbi.AllocationBase);
    printf("Allocation protect:           %s\n",   protect_to_str(mbi.AllocationProtect));
    printf("Region size:                  %zu bytes\n", mbi.RegionSize);
    printf("State:                        %s\n",   state_to_str(mbi.State));
    printf("Current protect:              %s\n",   protect_to_str(mbi.Protect));
    printf("Type:                         %s\n",   type_to_str(mbi.Type));
}

static void do_reserve_then_commit(void)
{
    char buf[64];
    SIZE_T size=0;
    LPVOID hint=NULL;

    printf("Enter region size in bytes: ");
    if(fgets(buf, sizeof(buf), stdin)==NULL){ printf("[ERROR] Input error!\n"); return; }
    size=(SIZE_T)strtoull(buf, NULL, 10);
    if(size==0){ printf("[ERROR] Invalid size.\n"); return; }

    printf("Enter starting address (0 - automatic): ");
    if(fgets(buf, sizeof(buf), stdin)==NULL){ printf("[ERROR] Input error!\n"); return; }
    hint=(LPVOID)(uintptr_t)strtoull(buf, NULL, 16);

    //резервирование региона без передачи физической памяти
    LPVOID reserved=VirtualAlloc(hint, size, MEM_RESERVE, PAGE_NOACCESS);
    if(reserved==NULL){
        printf("[ERROR] VirtualAlloc (MEM_RESERVE) failed with error: %lu\n", GetLastError());
        return;
    }
    printf("step 1: Region reserved. Address: %p, size: %zu bytes\n", reserved, size);

    //передача физической памяти зарезервированному региону
    LPVOID committed=VirtualAlloc(reserved, size, MEM_COMMIT, PAGE_READWRITE);
    if(committed==NULL){
        printf("[ERROR] VirtualAlloc (MEM_COMMIT) failed with error: %lu\n", GetLastError());
        VirtualFree(reserved, 0, MEM_RELEASE);
        return;
    }
    printf("step 2: Physical memory committed to region. Address: %p\n", committed);

    //освобождение памяти
    VirtualFree(committed, 0, MEM_RELEASE);
    printf("step 3: Region freed.\n");
}

static void do_reserve_and_commit(void)
{
    char buf[64];
    SIZE_T size=0;
    LPVOID hint=NULL;

    printf("Enter region size in bytes: ");
    if(fgets(buf, sizeof(buf), stdin)==NULL){ printf("[ERROR] Input error!\n"); return; }
    size=(SIZE_T)strtoull(buf, NULL, 10);
    if(size==0){ printf("[ERROR] Invalid size.\n"); return; }

    printf("Enter starting address (0 - automatic): ");
    if(fgets(buf, sizeof(buf), stdin)==NULL){ printf("[ERROR] Input error!\n"); return; }
    hint=(LPVOID)(uintptr_t)strtoull(buf, NULL, 16);

    //одновременное резервирование и выделение физической памяти
    LPVOID mem=VirtualAlloc(hint, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if(mem==NULL){
        printf("[ERROR] VirtualAlloc (MEM_RESERVE|MEM_COMMIT) failed with error: %lu\n", GetLastError());
        return;
    }
    printf("step 1: Region allocated. Address: %p, size: %zu bytes\n", mem, size);

    //освобождение памяти
    VirtualFree(mem, 0, MEM_RELEASE);
    printf("step 2: Region freed (VirtualFree).\n");
}

static void do_write_memory(void)
{
    char buf[256];
    LPVOID addr=NULL;

    printf("Enter starting address (0 - automatic): ");
    if(fgets(buf, sizeof(buf), stdin)==NULL){ printf("[ERROR] Input error!\n"); return; }
    addr=(LPVOID)(uintptr_t)strtoull(buf, NULL, 16);

    //Если адрес не указан — выделяем память автоматически
    int auto_alloc=0;
    if(addr==NULL){
        addr=VirtualAlloc(NULL, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if(addr==NULL){
            printf("[ERROR] VirtualAlloc failed with error: %lu\n", GetLastError());
            return;
        }
        auto_alloc=1;
        printf("Auto-allocated address: %p\n", addr);
    }

    printf("Enter string to write: ");
    if(fgets(buf, sizeof(buf), stdin)==NULL){ printf("[ERROR] Input error!\n"); return; }
    buf[strcspn(buf, "\n")]='\0';

    //Проверка доступности адреса
    MEMORY_BASIC_INFORMATION mbi;
    if(VirtualQuery(addr, &mbi, sizeof(mbi))==0){
        printf("[ERROR] VirtualQuery failed with error: %lu\n", GetLastError());
        if(auto_alloc) VirtualFree(addr, 0, MEM_RELEASE);
        return;
    }
    if(mbi.State != MEM_COMMIT){
        printf("[ERROR] Address %p is not committed (state: %s). Write is impossible.\n",
               addr, state_to_str(mbi.State));
        if(auto_alloc) VirtualFree(addr, 0, MEM_RELEASE);
        return;
    }
    if(mbi.Protect==PAGE_NOACCESS || mbi.Protect==PAGE_READONLY){
        printf("[ERROR] Address %p is write-protected (%s).\n", addr, protect_to_str(mbi.Protect));
        if(auto_alloc) VirtualFree(addr, 0, MEM_RELEASE);
        return;
    }

    //Запись данных
    memcpy(addr, buf, strlen(buf) + 1);
    printf("Written to address %p: \"%s\"\n", addr, (char *)addr);

    //Освобождение, если память была выделена автоматически
    if(auto_alloc){
        VirtualFree(addr, 0, MEM_RELEASE);
        printf("Memory freed.\n");
    }
}

static void do_virtual_protect(void)
{
    char buf[64];
    LPVOID addr=NULL;

    printf("Enter region address (hex, 0 - automatic): ");
    if(fgets(buf, sizeof(buf), stdin)==NULL){ printf("[ERROR] Input error!\n"); return; }
    addr=(LPVOID)(uintptr_t)strtoull(buf, NULL, 16);

    //Автоматическое выделение памяти, если адрес 0
    int auto_alloc=0;
    SIZE_T size=0;

    if(addr==NULL){
        printf("Enter region size in bytes: ");
        if(fgets(buf, sizeof(buf), stdin)==NULL){ printf("[ERROR] Input error!\n"); return; }
        size=(SIZE_T)strtoull(buf, NULL, 10);
        if(size==0){ printf("[ERROR] Invalid size.\n"); return; }

        addr=VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if(addr==NULL){
            printf("[ERROR] VirtualAlloc failed with error: %lu\n", GetLastError());
            return;
        }
        auto_alloc=1;
        printf("Auto-allocated address: %p, size: %zu bytes\n", addr, size);
    } else{
        printf("Enter region size in bytes: ");
        if(fgets(buf, sizeof(buf), stdin)==NULL){ printf("[ERROR] Input error!\n"); return; }
        size=(SIZE_T)strtoull(buf, NULL, 10);
        if(size==0){ printf("[ERROR] Invalid size.\n"); return; }
    }

    printf("     Choose protection type:\n");
    printf("(1) PAGE_READONLY\n");
    printf("(2) PAGE_READWRITE\n");
    printf("(3) PAGE_NOACCESS\n");
    printf("(4) PAGE_EXECUTE_READ\n");
    printf("Choose: ");
    if(fgets(buf, sizeof(buf), stdin)==NULL){ printf("[ERROR] Input error!\n"); return; }

    DWORD newProtect;
    switch (atoi(buf)){
    case 1: newProtect=PAGE_READONLY;       break;
    case 2: newProtect=PAGE_READWRITE;      break;
    case 3: newProtect=PAGE_NOACCESS;       break;
    case 4: newProtect=PAGE_EXECUTE_READ;   break;
    default:
        printf("[ERROR] Invalid choice.\n");
        if(auto_alloc) VirtualFree(addr, 0, MEM_RELEASE);
        return;
    }

    DWORD oldProtect=0;
    if(!VirtualProtect(addr, size, newProtect, &oldProtect)){
        printf("[ERROR] VirtualProtect failed with error: %lu\n", GetLastError());
        if(auto_alloc) VirtualFree(addr, 0, MEM_RELEASE);
        return;
    }
    printf("[INFO] Protection changed.\n");
    printf("       Old protection: %s\n", protect_to_str(oldProtect));
    printf("       New protection: %s\n", protect_to_str(newProtect));

    //Проверка через VirtualQuery
    MEMORY_BASIC_INFORMATION mbi;
    if(VirtualQuery(addr, &mbi, sizeof(mbi)) != 0)
        printf("[INFO] Verification: current protection=%s\n",
               protect_to_str(mbi.Protect));

    if(auto_alloc){
        VirtualFree(addr, 0, MEM_RELEASE);
        printf("[INFO] Auto-allocated memory freed.\n");
    }
}

static void print_menu(void)
{
    printf("\n    Menu:\n");
    printf("(1) System information\n");
    printf("(2) Virtual memory status\n");
    printf("(3) Memory region state\n");
    printf("(4) Separate reserve and commit\n");
    printf("(5) Simultaneous reserve and commit\n");
    printf("(6) Write data to address\n");
    printf("(7) Set and verify protection \n");
    printf("(0) Exit\n");
    printf("Choose an option: ");
}

int main(void)
{
    char line[64];
    int  choice;

    printf("[INFO] Program started. PID=%lu\n", GetCurrentProcessId());

    for (;;){
        print_menu();
        if(fgets(line, sizeof(line), stdin)==NULL)
            break;
        choice=atoi(line);

        switch (choice){
        case 1: do_system_info();         break;
        case 2: do_memory_status();       break;
        case 3: do_virtual_query();       break;
        case 4: do_reserve_then_commit(); break;
        case 5: do_reserve_and_commit();  break;
        case 6: do_write_memory();        break;
        case 7: do_virtual_protect();     break;
        case 0:
            printf("Program terminated.\n");
            return 0;
        default:
            printf("[ERROR] Invalid menu option!\n");
        }
    }

    return 0;
}