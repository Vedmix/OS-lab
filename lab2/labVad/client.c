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

static int   fd  = -1;         //файловый дескриптор
static char *ptr = MAP_FAILED; //указатель на отображённую память
static off_t filesize = 0;     //размер файла

static void do_map(void)
{
    if (ptr != MAP_FAILED) {
        printf("[ERROR] Файл уже спроецирован.\n");
        return;
    }

    fd = open(FILENAME, O_RDWR);
    if (fd == -1) {
        perror("open");
        printf("[ERROR] Убедитесь, что сервер создал файл '%s'.\n", FILENAME);
        return;
    }

    //получение реального размера файла
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        fd = -1;
        return;
    }
    filesize = st.st_size;

    //отображение файла в память
    ptr = mmap(NULL, (size_t)filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        close(fd);
        fd = -1;
        return;
    }

    printf("[INFO] Файл '%s' открыт и спроецирован в память.\n", FILENAME);
    printf("         Размер: %ld байт, адрес: %p\n", (long)filesize, (void *)ptr);
}

static void do_read(void)
{
    if (ptr == MAP_FAILED) {
        printf("[ERROR] Сначала выполните проецирование (пункт 1).\n");
        return;
    }

    int ack_offset = (int)filesize - 1;

    //проверка первого байта (если равен '\0', то сервер ничего не записал)
    printf("[INFO] Ожидание данных от сервера...\n");

    struct timeval tv;
    int waited = 0;
    while (ptr[0] == '\0') {
        tv.tv_sec  = 0;
        tv.tv_usec = 100000; // 100 мс

        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        select(fd + 1, &rfds, NULL, NULL, &tv);

        waited++;
        if (waited % 20 == 0)
            printf("[INFO] Время ожидания: %d с\n", waited / 10);
        if (waited > 300) {   //30 секунд
            printf("[INFO] Таймаут ожидания сервера.\n");
            return;
        }
    }

    //Читаем строку из общей памяти (без последнего байта — там флаг)
    char buf[4096];
    int len = ack_offset < (int)sizeof(buf) ? ack_offset : (int)sizeof(buf) - 1;
    strncpy(buf, ptr, len);
    buf[len] = '\0';

    printf("[INFO] Получено: \"%s\"\n", buf);

    //Отправляем подтверждение серверу, записываем '\1' в последний байт
    ptr[ack_offset] = '\1';
    if (msync(ptr, (size_t)filesize, MS_SYNC) == -1)
        perror("msync");

    printf("[INFO] Подтверждение отправлено серверу.\n");
}

static void do_exit(void)
{
    if (ptr != MAP_FAILED) {
        munmap(ptr, (size_t)filesize);
        ptr      = MAP_FAILED;
        filesize = 0;
        printf("[INFO] Проецирование отменено.\n");
    }
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
    printf("[INFO] Работа завершена.\n");
    exit(EXIT_SUCCESS);
}

static void print_menu(void)
{
    printf("\n    Меню клиента:\n");
    printf("(1) Выполнить проецирование\n");
    printf("(2) Прочитать данные\n");
    printf("(3) Завершить работу\n");
    printf("Выберите пункт: ");
}

int main(void)
{
    char line[64];
    int  choice;

    printf("[INFO] Клиент запущен. PID = %dы", getpid());

    for (;;) {
        print_menu();
        if (fgets(line, sizeof(line), stdin) == NULL)
            break;
        choice = atoi(line);

        switch (choice) {
        case 1: do_map();   break;
        case 2: do_read();  break;
        case 3: do_exit();  break;
        default:
            printf("[ERROR] Неверный пункт меню.\n");
        }
    }

    do_exit();
    return 0;
}
