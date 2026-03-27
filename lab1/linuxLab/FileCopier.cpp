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
    //извлекаем имя файла из srcPath и склеиваем с destDir
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

    //O_DIRECT требует чтобы размер блока был кратен ALIGN
    //округляем fileSize вверх до кратного ALIGN для последнего блока
    //это нужно чтобы последний неполный блок тоже читался корректно
    off_t alignedSize = ((fileSize + ALIGN - 1) / ALIGN) * ALIGN;

    //Выделение выровненных буферов
    //два отдельных буфера: один для чтения, другой для записи
    void* readBuf  = nullptr;
    void* writeBuf = nullptr;
    posix_memalign(&readBuf,  ALIGN, blockSize);
    posix_memalign(&writeBuf, ALIGN, blockSize);

    //Инициализация контекста AIO (асинхронные операции чтения и записи)
    //io_context_t - дескриптор очереди асинхронных операций
    //io_setup - сколько операций одновременно можно выполнять (в данном случае 2 - запись и чтение)
    io_context_t ctx = {};
    io_setup(2, &ctx);

    off_t readOffset = 0; //текущая позиция чтения в исходном файле
    off_t writeOffset = 0; //текущая позиция записи в файле назначения
    size_t bytesRead = 0; //сколько байт уже прочитано (готово к записи)
    int inFlight = 0; //счётчик операций находящихся сейчас в очереди ядра

    //iocb - структура описания одной асинхронной операции (I/O Control Block)
    //readCb - описывает текущую операцию чтения
    //writeCb - описывает текущую операцию записи
    struct iocb readCb, writeCb;
    struct iocb* cbs[2]; //для передачи в io_submit

    //До начала основного цикла вручную запускаем первую операцию чтения, чтобы к первой итерации цикла данные уже были в readBuf

    //toRead — либо полный блок, либо сколько осталось до конца выровненного файла

    size_t toRead = std::min((off_t)blockSize, alignedSize - readOffset);
    io_prep_pread(&readCb, srcFd, readBuf, toRead, readOffset); //заполняет структуру iocb для операции чтения count байт из fd начиная со смещения offset в буфер buf
    cbs[0] = &readCb;

    int rc = io_submit(ctx, 1, cbs); //отправляет n операций в очередь ядра
    if(rc < 0){
        io_destroy(ctx);
        free(readBuf); free(writeBuf);
        close(srcFd); close(dstFd);
        return false;
    }
    readOffset += toRead;
    inFlight++;

    while (inFlight > 0) {
        //io_event — структура результата завершившейся операции
        struct io_event events[2];

        //минимум 1 событие, максимум inFlight событий
        //timeout = nullptr - ждём бесконечно
        //поток спит до прихода completion event
        int nEvents = io_getevents(ctx, 1, inFlight, events, nullptr); //возвращает реальное число завершившихся операций
        if (nEvents < 0) {
            break;
        }

        inFlight -= nEvents; //уменьшаем счётчик на число завершившихся операций

        for (int i = 0; i < nEvents; i++) {
            struct iocb* completed = events[i].obj; //какая именно операция завершилась

            //events[i].res — результат операции (число обработанных байт. Если < 0, значит была ошибка)
            if (events[i].res < 0) {
                continue;
            }

            if (completed == &readCb) {
                //реально прочитанных байт — не больше чем размер файла
                //ограничиваем чтобы не записать лишние байты padding'а в конец файла
                bytesRead = std::min((off_t)events[i].res, fileSize - writeOffset);

                //меняем буферы местами без копирования данных:
                //то что прочитали (readBuf) идёт на запись (станет writeBuf)
                //свободный writeBuf получит следующий блок чтения (станет readBuf)
                std::swap(readBuf, writeBuf);

                //округляем вверх
                size_t toWrite = ((bytesRead + ALIGN - 1) / ALIGN) * ALIGN;

                //заполняет структуру iocb для операции записи count байт из buf в fd начиная со смещения offset
                io_prep_pwrite(&writeCb, dstFd, writeBuf, toWrite, writeOffset);

                int toSubmit = 0;
                cbs[toSubmit++] = &writeCb; //запись - всегда ставим в очередь

                //если файл не кончился - сразу читаем следующий блок
                //не дожидаясь завершения записи
                if (readOffset < alignedSize) {
                    toRead = std::min((off_t)blockSize, alignedSize - readOffset);
                    io_prep_pread(&readCb, srcFd, readBuf, toRead, readOffset);
                    cbs[toSubmit++] = &readCb; //чтение
                    readOffset += toRead;
                }

                //отправляем обе операции одним вызовом io_submit
                rc = io_submit(ctx, toSubmit, cbs);
                if (rc < 0){
                    inFlight += toSubmit;
                }

            } else if (completed == &writeCb) {
                //двигаем указатель на реальный размер прочитанных данных,
                writeOffset += bytesRead;
            }
        }
    }

    //Очистка
    //io_destroy освобождает контекст AIO и все связанные ресурсы ядра
    io_destroy(ctx);

    free(readBuf);
    free(writeBuf);
    close(srcFd);
    close(dstFd);

    //обрезаем файл до реального размера — убираем нулевой padding
    //который появился из-за выравнивания последнего блока до кратного ALIGN
    truncate(destPath.c_str(), fileSize);

    return true;
}