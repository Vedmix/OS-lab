#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAPNAME     "Local\\SharedFileMap"
#define FILESIZE    4096            //размер файла в байтах
#define ACK_OFFSET (FILESIZE - 1) //смещение байта-подтверждения

static HANDLE hMapping  = NULL; //дескриптор отображения
static char  *ptr       = NULL; //указатель на отображённую память


static void do_map(void)
{
    if(ptr ! =  NULL){
        printf("[ERROR] Файл уже спроецирован.\n");
        return;
    }

    //открытие существующего объекта отображения по имени
    hMapping = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, MAPNAME);
    if(hMapping == NULL){
        printf("[ERROR] OpenFileMapping завершился с ошибкой: %lu\n", GetLastError());
        printf("[ERROR] Убедитесь, что сервер выполнил проецирование.\n");
        return;
    }

    //проецирование файла в адресное пространство процесса
    ptr  = (char *)MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, FILESIZE);
    if(ptr == NULL){
        printf("[ERROR] MapViewOfFile завершился с ошибкой: %lu\n", GetLastError());
        CloseHandle(hMapping);
        hMapping = NULL;
        return;
    }

    printf("[INFO] Файл спроецирован в память.\n");
    printf("       Имя отображения: %s\n", MAPNAME);
    printf("       Адрес отображения: %p\n",(void *)ptr);
}

static void do_read(void)
{
    if(ptr == NULL){
        printf("[ERROR] Сначала выполните проецирование(пункт 1).\n");
        return;
    }

    //проверка первого байта(если равен '\0', то сервер ничего не записал)
    printf("[INFO] Ожидание данных от сервера...\n");

    int waited = 0;
    while(ptr[0] == '\0'){
        Sleep(100); //100 мс

        waited++;
        if(waited % 20 == 0)
            printf("[INFO] Время ожидания: %d с\n", waited / 10);
        if(waited > 300){   //30 секунд
            printf("[INFO] Таймаут ожидания сервера.\n");
            return;
        }
    }

    //чтение строки из общей памяти(без последнего байта — там флаг)
    char buf[FILESIZE];
    int len = ACK_OFFSET <(int)sizeof(buf)? ACK_OFFSET :(int)sizeof(buf)- 1;
    strncpy(buf, ptr, len);
    buf[len] = '\0';

    printf("[INFO] Получено: \"%s\"\n", buf);

    //отправка подтверждения серверу, записываем '\1' в последний байт
    ptr[ACK_OFFSET] = '\1';
    if(!FlushViewOfFile(ptr, FILESIZE))
        printf("[ERROR] FlushViewOfFile: %lu\n", GetLastError());

    printf("[INFO] Подтверждение отправлено серверу.\n");
}

static void do_exit(void)
{
    if(ptr ! =  NULL){
        //отмена проецирования
        UnmapViewOfFile(ptr);
        ptr = NULL;
        printf("[INFO] Проецирование отменено.\n");
    }
    if(hMapping ! =  NULL){
        CloseHandle(hMapping);
        hMapping = NULL;
    }

    printf("[INFO] Работа завершена.\n");
    exit(EXIT_SUCCESS);
}

static void print_menu(void)
{
    printf("\n    Меню клиента:\n");
    printf("(1)Выполнить проецирование\n");
    printf("(2)Прочитать данные\n");
    printf("(3)Завершить работу\n");
    printf("Выберите пункт: ");
}

int main(void)
{
    char line[64];
    int  choice;

    printf("[INFO] Клиент запущен. PID = %lu\n", GetCurrentProcessId());

    for(;;){
        print_menu();
        if(fgets(line, sizeof(line), stdin) ==  NULL)
            break;
        choice = atoi(line);

        switch(choice){
        case 1: do_map();   break;
        case 2: do_read();  break;
        case 3: do_exit();  break;
        default:
            printf("[ERROR] Неверный пункт меню!\n");
        }
    }

    do_exit();
    return 0;
}
