#include "TestCopying.hpp"

TestCopying::TestCopying(){
    numTests=0;
    fileSize=0;
}
TestCopying::~TestCopying(){}

void TestCopying::doTests(const int _numTests, const FileCopier& _fileCpy, const size_t _fileSize, const int numCycles){
    numTests=_numTests;
    fileCpy=_fileCpy;
    fileSize=_fileSize*1024;
    double totalRuntime, avrgRuntime;
    std::cout << "Тестирование...\n";
    for(int i=0;i<numCycles;i++){
        totalRuntime+=static_cast<double>(doOneCycleTest());
    }
    avrgRuntime = totalRuntime/numCycles;
    std::cout << "Среднее время выполнения (" << numCycles << " циклов): " << avrgRuntime <<" мс"<< std::endl;
}

int TestCopying::doOneCycleTest(){
    if(!std::filesystem::create_directories("filesOrig")){
        deleteAllFilesInDir("filesOrig");
    }
    if(!std::filesystem::create_directories("filesCopy")){
        deleteAllFilesInDir("filesCopy");
    }
    createFilesForTest();
    unsigned int runtime;
    auto start = std::chrono::high_resolution_clock::now();
    doCopying();
    auto end = std::chrono::high_resolution_clock::now();
    runtime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    return runtime;
}

void TestCopying::deleteAllFilesInDir(const std::string& dst){
    for(const auto& entry : std::filesystem::directory_iterator(dst)){
        if(std::filesystem::is_regular_file(entry.status())){
            std::filesystem::remove(entry.path());
        }
    }
}

void TestCopying::createFilesForTest(){
    for(int i=0;i<numTests;i++){
        createTestFile(std::to_string(i) + ".txt");
    }
}

void TestCopying::createTestFile(const std::string& filename){
    std::vector<char> buffer(fileSize, 0);
    std::ofstream file("filesOrig/"+filename);
    
    if(file.is_open()){
        for(size_t i=0; i<fileSize;i++){
            file << '0';
        }
        file.close();
    } else{
        std::cout << "Ошибка: не удалось создать файл " << filename << std::endl;
    }
}

void TestCopying::doCopying(){
    for(const auto& entry : std::filesystem::directory_iterator("filesOrig")){
        if(std::filesystem::is_regular_file(entry.status())){
            fileCpy.copyFile(entry.path(), "filesCopy");
        }
    }
}