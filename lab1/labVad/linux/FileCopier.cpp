#include "FileCopier.hpp"

FileCopier::FileCopier(){}

FileCopier::FileCopier(const size_t _ALIGN, const int blockSizeMultiplier):ALIGN(_ALIGN*1024){
    blockSize = ALIGN*blockSizeMultiplier;
    overlapCount=1;
}

FileCopier::~FileCopier(){}

void FileCopier::setAlign(const size_t _ALIGN, const int blockSizeMultiplier){
    ALIGN=_ALIGN*1024;
    blockSize=ALIGN*blockSizeMultiplier;
}

void FileCopier::setOverlapCount(const int _overlapCount){
    overlapCount=_overlapCount;
}

size_t FileCopier::getAlign(){return ALIGN;}
size_t FileCopier::getBlockSize(){return blockSize;}
int FileCopier::getOverlapCount(){return overlapCount;}

FileCopier& FileCopier::operator=(const FileCopier& _fileCpy){
    if(this==&_fileCpy){
        return *this;
    }
    ALIGN=_fileCpy.ALIGN;
    blockSize=_fileCpy.blockSize;
    overlapCount=_fileCpy.overlapCount;
    return *this;
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
    off_t alignedSize = ((fileSize+ALIGN-1) / ALIGN) * ALIGN;

    //Выделение массивов буферов и iocb по n штук
    //каждый слот i имеет свой readBuf[i], writeBuf[i], readCb[i], writeCb[i]
    //это позволяет n операциям чтения и n операциям записи работать параллельно
    std::vector<void*> readBufs(overlapCount, nullptr);
    std::vector<void*> writeBufs(overlapCount, nullptr);
    for(int i=0; i<overlapCount; i++){
        posix_memalign(&readBufs[i],  ALIGN, blockSize);
        posix_memalign(&writeBufs[i], ALIGN, blockSize);
    }

    //iocb — структура описания одной асинхронной операции (I/O Control Block)
    //по n штук для чтения и записи - каждый слот независим
    std::vector<iocb> readCbs(overlapCount);
    std::vector<iocb> writeCbs(overlapCount);

    //bytesRead[i] - сколько байт прочитал слот i (нужно при завершении записи)
    std::vector<size_t> bytesReadArr(overlapCount, 0);

    //Инициализация контекста AIO
    //io_context_t - дескриптор очереди асинхронных операций
    //io_setup - максимум 2*n операций одновременно
    io_context_t ctx = {};
    io_setup(2*overlapCount, &ctx);

    off_t readOffset  = 0;  //текущая позиция чтения в исходном файле
    off_t writeOffset = 0;  //текущая позиция записи в файле назначения
    int inFlight = 0;  //счётчик операций находящихся сейчас в очереди ядра

    struct iocb* cbs[2];
    int rc;

    //Запускаем первые n операций чтения чтобы заполнить очередь
    for(int i=0; i<overlapCount && readOffset<alignedSize; i++){
        size_t toRead = std::min((off_t)blockSize, alignedSize - readOffset);

        //заполняет структуру iocb для операции чтения count байт из fd начиная со смещения offset в буфер buf
        io_prep_pread(&readCbs[i], srcFd, readBufs[i], toRead, readOffset);

        //сохраняем индекс слота в поле data, чтобы в цикле понять, какой именно слот завершился когда придёт событие
        readCbs[i].data = (void*)(intptr_t)i;

        cbs[0] = &readCbs[i];

        //отправляет n операций в очередь ядра, после этого вызова операция выполняется асинхронно
        rc = io_submit(ctx, 1, cbs);
        if(rc<0){
            break;
        }
        readOffset += toRead;
        inFlight++;
    }

    //Основной цикл перекрывающегося копирования
    while(inFlight>0){
        struct io_event events[2*overlapCount];

        //минимум 1 событие, максимум inFlight событий
        //поток спит до прихода completion event
        int nEvents = io_getevents(ctx, 1, inFlight, events, nullptr);
        if(nEvents<0){
            break;
        }

        inFlight -= nEvents; //уменьшаем счётчик на число завершившихся операций

        for(int i=0; i<nEvents; i++){
            struct iocb* completed = events[i].obj; //какая именно операция завершилась

            //получаем индекс слота из поля data которое мы сами туда записали
            int slot = (int)(intptr_t)completed->data;

            //число обработанных байт
            if(events[i].res < 0){
                continue;
            }

            if(completed == &readCbs[slot]){
                //Чтение слота завершено
                //реально прочитанных байт — не больше чем размер файла
                bytesReadArr[slot] = std::min((off_t)events[i].res, fileSize - writeOffset);

                //меняем буферы слота местами без копирования:
                std::swap(readBufs[slot], writeBufs[slot]);

                //toWrite тоже должен быть кратен ALIGN для O_DIRECT, округляем вверх
                size_t toWrite = ((bytesReadArr[slot] + ALIGN - 1) / ALIGN) * ALIGN;

                //заполняет структуру iocb для операции записи count байт из buf в fd начиная со смещения offset
                io_prep_pwrite(&writeCbs[slot], dstFd, writeBufs[slot], toWrite, writeOffset);

                //сохраняем индекс слота чтобы при завершении записи, чтобы знать какой слот освободился
                writeCbs[slot].data = (void*)(intptr_t)slot;
                writeOffset += bytesReadArr[slot];

                int toSubmit = 0;
                cbs[toSubmit++] = &writeCbs[slot]; //запись

                //если файл не кончился - сразу читаем следующий блок в этот же слот
                if(readOffset < alignedSize){
                    size_t toRead = std::min((off_t)blockSize, alignedSize - readOffset);
                    io_prep_pread(&readCbs[slot], srcFd, readBufs[slot], toRead, readOffset);
                    readCbs[slot].data = (void*)(intptr_t)slot;
                    cbs[toSubmit++] = &readCbs[slot];
                    readOffset += toRead;
                }

                //отправляем обе операции одним вызовом io_submit
                rc = io_submit(ctx, toSubmit, cbs);
                if(rc < 0)
                    std::cout << "io_submit failed: " << strerror(-rc) << std::endl;
                else
                    inFlight += toSubmit;

            }
        }
    }

    //Очистка
    //io_destroy освобождает контекст AIO и все связанные ресурсы ядра
    //важно вызывать после того как все операции завершены
    io_destroy(ctx);

    for(int i = 0; i < overlapCount; i++){
        free(readBufs[i]);
        free(writeBufs[i]);
    }
    close(srcFd);
    close(dstFd);

    //обрезаем файл до реального размера - убираем нулевой padding
    truncate(destPath.c_str(), fileSize);

    return true;
}