#include "TestCopying.hpp"

TestCopying::TestCopying(){
    numTests=0;
}
TestCopying::~TestCopying(){}

void TestCopying::doTests(const int _numTests, const FileCopier& _fileCpy){
    numTests=_numTests;
    fileCpy=_fileCpy;
    deleteAllFilesInDir("filesOrig");
    deleteAllFilesInDir("filesCopy");
    createFilesForTest();
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
    const size_t SIZE_64KB = 64*1024;
    std::vector<char> buffer(SIZE_64KB, 0);
    std::ofstream file("filesOrig/"+filename);
    
    if(file.is_open()){
        for(size_t i=0; i<SIZE_64KB;i++) {
            file << '0';
        }
        file.close();
    } else{
        std::cout << "Ошибка: не удалось создать файл " << filename << std::endl;
    }
}