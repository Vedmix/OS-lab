#include "tools.hpp"

string trimmed(const string& str){
    size_t first = str.find_first_not_of(" \t\n\r");
    if(first == string::npos)return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}


void printFileTimeForTime(const FILETIME& ftime,const string& labe){
        if(ftime.dwHighDateTime == 0 && ftime.dwLowDateTime ==0){
            cout<<labe<<": Not avalible"<<endl;
            return;
        }
        SYSTEMTIME stime;
        FileTimeToSystemTime(&ftime,&stime);

        cout<<labe<<": "<<std::setfill('0')<<std::setw(2)<<stime.wDay<<"."<<std::setw(2)<<stime.wMonth<<"."<<std::setw(4)<<stime.wYear<<" "<<std::setw(2)<<stime.wHour<<":"<<std::setw(2)<<stime.wMinute<<":"<<std::setw(2)<<stime.wSecond<<std::setfill(' ')<<endl;
    }

void printFileTimeForHandle(const FILETIME& ft, const char* label){
	SYSTEMTIME st;
	FileTimeToSystemTime(&ft, &st);
	std::cout<<label<<": "<<std::setfill('0')<< st.wDay<<"."<<std::setw(2)<<st.wMonth<<"."<<st.wYear<<" "<< std::setw(2)<<st.wHour<<":"<<std::setw(2)<<st.wMinute<<":"<<std::setw(2)<<st.wSecond<<std::endl;
}


bool checkPath(const string& path, bool& isDirectory){
    DWORD attrs = GetFileAttributes(path.c_str());
    if(attrs == INVALID_FILE_ATTRIBUTES){
        return false;
    }
    isDirectory = (attrs & FILE_ATTRIBUTE_DIRECTORY)!= 0;
    return true;
}

string getFileName(const string& path){
    size_t pos = path.find_last_of("\\");
    if(pos != string::npos){
        return path.substr(pos + 1);
    }
    return path;
}

string getCopyFileName(const string& originalPath){
    string fileName = getFileName(originalPath);
    size_t dotPos = fileName.find_last_of(".");
    
    if(dotPos != string::npos){
        string nameWithoutExt = fileName.substr(0, dotPos);
        string ext = fileName.substr(dotPos);
        return nameWithoutExt + "_copy" + ext;
    }else{
        return fileName + "_copy";
    }
}

bool checkContain(const string& fullPath){
    string searchPath = fullPath + "\\*";
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);
    
   if(hFind == INVALID_HANDLE_VALUE){
        return false;  
    }
    
    do{
       if(strcmp(findData.cFileName, ".") != 0 && 
            strcmp(findData.cFileName, "..") != 0){
            FindClose(hFind);
            return true;  
        }
    } while (FindNextFile(hFind, &findData));
    
    FindClose(hFind);
    return false;  
}