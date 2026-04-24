#define _GNU_SOURCE
#include <iostream>
#include <aio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <chrono>

struct aio_operation {
    aiocb aio;
    char* buffer;
    int flag;
    int id;
    int write_fd;
};

void aio_completion_handler(sigval_t sigval){
    aio_operation* op = (aio_operation*)sigval.sival_ptr;

    if(op->flag == 3){
        ssize_t bytes = aio_return(&op->aio);

        if(bytes <= 0){
            op->flag = -1;
            return;
        }

        op->aio.aio_fildes = op->write_fd;
        op->aio.aio_nbytes = bytes;
        aio_write(&op->aio);

        op->flag = 2;
    }
    else if(op->flag == 2){
        aio_return(&op->aio);
        op->flag = -1;
    }
}

void start(std::vector<aio_operation>& buffs, int block_size, int read_fd){
    for(int i = 0; i < buffs.size(); i++){
        buffs[i].aio.aio_fildes = read_fd;
        buffs[i].aio.aio_offset = i * block_size;
        buffs[i].aio.aio_nbytes = block_size;
        buffs[i].flag = 3;

        aio_read(&buffs[i].aio);
    }
}

bool isDone(std::vector<aio_operation>& buffs){
    for(auto &b : buffs){
        if(b.flag != -1) return false;
    }
    return true;
}

int main(){
    int read_fd, write_fd;
    int block_size = 65536;
    int num;
    std::string temp;

    std::vector<aio_operation> buffs;
    int menu;

    while(true){
        std::cout<<"=====MENU====="<<std::endl;
        std::cout<<"1.Set params"<<std::endl;
        std::cout<<"2.Run test"<<std::endl;
        std::cout<<"0.Exit"<<std::endl;

        getline(std::cin, temp);
        menu = stoi(temp);

        switch(menu){

            case 1:
            {
                std::cout<<"Enter number of operations: ";
                getline(std::cin, temp);
                num = stoi(temp);

                std::cout<<"Enter block size: ";
                getline(std::cin, temp);
                block_size = stoi(temp);

                buffs.clear();

                for(int i = 0; i < num; i++){
                    aio_operation buf;

                    buf.buffer = new char[block_size];
                    buf.aio.aio_buf = buf.buffer;
                    buf.id = i;
                    buf.flag = 0;

                    buf.aio.aio_sigevent.sigev_notify = SIGEV_THREAD;
                    buf.aio.aio_sigevent.sigev_notify_function = aio_completion_handler;

                    buffs.push_back(buf);
                }
            }
            break;

            case 2:
            {
                read_fd = open("archive", O_RDONLY);
                write_fd = open("target", O_CREAT | O_WRONLY | O_TRUNC, 0666);

                if(read_fd == -1 || write_fd == -1){
                    std::cout<<"File open error"<<std::endl;
                    break;
                }

                for(int i = 0; i < buffs.size(); i++){
                    buffs[i].write_fd = write_fd;
                    buffs[i].aio.aio_sigevent.sigev_value.sival_ptr = &buffs[i];
                }

                auto start_time = std::chrono::high_resolution_clock::now();

                start(buffs, block_size, read_fd);

                while(!isDone(buffs)){
                    usleep(1000);
                }

                auto end_time = std::chrono::high_resolution_clock::now();
                auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

                std::cout<<"Time: "<<time.count()<<" ms"<<std::endl;

                close(read_fd);
                close(write_fd);
            }
            break;

            case 0:
                return 0;

            default:
                break;
        }
    }
} 