#include "FileCopier.hpp"

FileCopier::FileCopier(){}

FileCopier::FileCopier(const size_t _ALIGN, const int blockSizeMultiplier):ALIGN(_ALIGN*1024){
    blockSize = ALIGN*blockSizeMultiplier;
}

FileCopier::~FileCopier(){}

void FileCopier::setAlign(const size_t _ALIGN, const int blockSizeMultiplier){
    ALIGN=_ALIGN*1024;
    blockSize=ALIGN*blockSizeMultiplier;
}

bool FileCopier::copyFile(const std::string& srcPath, const std::string& destDir){
    std::string filename = std::filesystem::path(srcPath).filename();
    std::string destPath  = std::filesystem::path(destDir) / filename;
    //открытие файла
    //O_RDONLY - только чтение
    //O_DIRECT - обход page cache: данные идут напрямую диск - буфер
    int srcFd = open(srcPath.c_str(), O_RDONLY | O_DIRECT);
    if(srcFd<0){
        std::cout << "Не удалось открыть исходный файл: " + srcPath;
        return false;
    }

    //O_WRONLY - только запись
    //O_CREAT - создать файл если не существует
    //O_TRUNC - обнулить файл если существует
    //O_DIRECT - тот же смысл, что и для источника
    //0644 устанавливает права для чтения и записи (6 - владелец и читать и писать, остальные 4 - только читать) 
    int dstFd = open(destPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_DIRECT, 0644);
    if(dstFd<0){
        std::cout << ("Не удалось создать файл назначения: " + destPath);
        close(srcFd);
        return false;
    }

    //Узнаём размер исходного файла
    struct stat fileStat;
    fstat(srcFd, &fileStat);
    off_t fileSize = fileStat.st_size;

    //Выделение выровненных буферов
    void* readBuf  = nullptr;
    void* writeBuf = nullptr;
    posix_memalign(&readBuf,  ALIGN, blockSize);
    posix_memalign(&writeBuf, ALIGN, blockSize);

    //Инициализация контекста AIO (асинхронные операции чтения и записи)
    //io_context_t - дескриптор очереди асинхронных операций.
    //io_setup - сколько операций одновременно можно выполнять (в данном случае 2 - запись и чтение)
    io_context_t ctx = {};
    io_setup(2, &ctx);

    off_t readOffset = 0;   //текущая позиция чтения в исходном файле
    off_t writeOffset = 0;   //текущая позиция записи в файле назначения
    size_t bytesRead = 0;   //сколько байт уже прочитано (готово к записи)
    int inFlight = 0;
    //Первое чтение
    //До начала основного цикла вручную запускаем первую операцию чтения,
    //чтобы к первой итерации цикла данные уже были в readBuf

    //iаocb — структур описания одной асинхронной операции (I/O Control Block)

    //io_prep_pread(iocb, fd, buf, count, offset):
    //заполняет структуру iocb для операции чтения count байт
    //из fd начиная со смещения offset в буфер buf
    struct iocb readCb, writeCb;
    struct iocb* cbs[1];

    size_t toRead = std::min((off_t)blockSize, fileSize - readOffset); //либо сколько осталось до конца файла, либо полный блок
    io_prep_pread(&readCb, srcFd, readBuf, toRead, readOffset);
    cbs[0] = &readCb;

    io_submit(ctx, 1, cbs); //отправляет n операций в очередь ядра
    readOffset += toRead;
    inFlight++;

    //Основной цикл перекрывающегося копирования
    while(writeOffset < fileSize){

        //io_event — структура результата завершившейся операции
        //Массив на 2 элемента: ждём завершения чтения (и записи, если шла)
        struct io_event events[2];

        //от 1 до 2 событий
        //nullptr - ждём бесконечно
        int nEvents = io_getevents(ctx, 1, inFlight, events, nullptr); //поток спит до прихода completion event, возвращает реальное число завершившихся операций
        inFlight -= nEvents;

        for(int i=0; i<nEvents; i++){
            struct iocb* completed = events[i].obj;

            if(completed == &readCb){
                //Чтение завершено
                bytesRead = events[i].res; //res - реально прочитанных байт

                //Меняем буферы местами: то что прочитали идёт на запись, свободный writeBuf получит следующий блок чтения
                std::swap(readBuf, writeBuf);

                //Запускаем запись прочитанного блока
                io_prep_pwrite(&writeCb, dstFd, writeBuf, bytesRead, writeOffset);
                cbs[0] = &writeCb;
                io_submit(ctx, 1, cbs);
                inFlight++;

                //Если файл не кончился — сразу запускаем следующее чтение, не дожидаясь завершения записи (запись предыдущего блока и чтение следующего идут параллельно)
                if(readOffset < fileSize){
                    toRead = std::min((off_t)blockSize, fileSize - readOffset);
                    io_prep_pread(&readCb, srcFd, readBuf, toRead, readOffset);
                    cbs[0] = &readCb;
                    io_submit(ctx, 1, cbs);
                    readOffset += toRead;
                    inFlight++;
                }

            } else if(completed == &writeCb){
                //Запись завершена
                writeOffset += events[i].res; //Просто продвигаем указатель - следующая запись будет запущена когда придёт следующее событие чтения
            }
        }
    }

    //Очистка
    //io_destroy освобождает контекст AIO и все связанные ресурсы ядра.
    io_destroy(ctx);

    free(readBuf);
    free(writeBuf);
    close(srcFd);
    close(dstFd);

    return true;
}