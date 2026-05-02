#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <errno.h>

#define FILENAME  "shared_file.dat"
#define FILESIZE  4096          //размер файла в байтах

static int   fd  = -1;         //файловый дескриптор
static char *ptr = MAP_FAILED; //указатель на отображённую память


//Инициализация файла нужного размера
static int init_file(void)
{
    //Заполнение файла нулями до нужного размера
    char buf[FILESIZE];
    memset(buf, 0, sizeof(buf));
    if (lseek(fd, 0, SEEK_SET) == -1 ||
        write(fd, buf, sizeof(buf)) != sizeof(buf)) {
        perror("init_file: write");
        return -1;
    }
    return 0;
}

static void do_map(void)
{
    if (ptr != MAP_FAILED) {
        printf("[ERROR] Файл уже спроецирован.\n");
        return;
    }

    fd = open(FILENAME, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open");
        return;
    }

    if (init_file() != 0) {
        close(fd);
        fd = -1;
        return;
    }

    ptr = mmap(NULL, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        close(fd);
        fd = -1;
        return;
    }

    printf("[INFO] Файл '%s' создан и спроецирован в память.\n", FILENAME);
    printf("         Адрес отображения: %p\n", (void *)ptr);
}

static void do_write(void)
{
    if (ptr == MAP_FAILED) {
        printf("[ERROR] Сначала выполните проецирование!\n");
        return;
    }

    char message[FILESIZE];
    printf("[Сервер] Введите строку для записи: ");
    if (fgets(message, sizeof(message), stdin) == NULL) {
        printf("[ERROR] Ошибка ввода!\n");
        return;
    }
    //удаление перевода строки
    message[strcspn(message, "\n")] = '\0';

    //Запись строки в начало отображённой памяти
    strncpy(ptr, message, FILESIZE - 1);
    ptr[FILESIZE - 1] = '\0';

    //Сброс изменения на диск
    if (msync(ptr, FILESIZE, MS_SYNC) == -1)
        perror("msync");

    printf("[INFO] Записано: \"%s\"\n", ptr);
    printf("[INFO] Ожидание клиента...\n");

    //Ожидание подтверждения от клиента('\1' в конеце буфера)
    int ack_offset = FILESIZE - 1;
    ptr[ack_offset] = '\0';   //сброс флаг подтверждения

    //Опрос с таймаутом 100 мс
    struct timeval tv;
    int waited = 0;
    //(ptr[ack_offset]) сигнал о прочтении
    while (ptr[ack_offset] != '\1') {
        tv.tv_sec  = 0;
        tv.tv_usec = 100000; //100 мс

        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        //select() используется только для задержки
        select(fd + 1, NULL, NULL, NULL, &tv);

        waited++;
        if (waited % 20 == 0)
            printf("[INFO] Ожидание подтверждения от клиента... (%d с)\n", waited / 10);
        if (waited > 300) {   // 30 секунд
            printf("[INFO] Таймаут ожидания клиента.\n");
            return;
        }
    }
    printf("[INFO] Клиент подтвердил прочтение.\n");
}

static void do_exit(void)
{
    if (ptr != MAP_FAILED) {
        munmap(ptr, FILESIZE);
        ptr = MAP_FAILED;
        printf("[INFO] Проецирование отменено.\n");
    }
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
    if (unlink(FILENAME) == 0)
        printf("[INFO] Файл '%s' удалён.\n", FILENAME);
    else if (errno != ENOENT)
        perror("unlink");

    printf("[INFO] Работа завершена.\n");
    exit(EXIT_SUCCESS);
}


static void print_menu(void)
{
    printf("\n    Меню сервера:\n");
    printf("(1)Выполнить проецирование\n");
    printf("(2)Записать данные\n");
    printf("(3)Завершить работу\n");
    printf("Выберите пункт: ");
}

int main(void)
{
    char line[64];
    int  choice;

    printf("[INFO] Сервер запущен. PID = %d\n", getpid());

    for (;;) {
        print_menu();
        if (fgets(line, sizeof(line), stdin) == NULL)
            break;
        choice = atoi(line);

        switch (choice) {
        case 1: do_map();   break;
        case 2: do_write(); break;
        case 3: do_exit();  break;
        default:
            printf("[ERROR] Неверный пункт меню!\n");
        }
    }

    do_exit();
    return 0;
}
