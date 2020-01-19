#ifndef WIN
#define WIN
#include <Windows.h>
//#include <windows.h>
using std::string;

void get_files_folder(string folder, vector<string> * names)
{
    string search_path = folder + "/*.*";
    WIN32_FIND_DATA fd; 
    HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd); 
    if(hFind != INVALID_HANDLE_VALUE) { 
        do { 
            // read all (real) files in current folder
            // , delete '!' read other 2 default folder . and ..
            if(! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
                names->push_back(fd.cFileName);
            }
        }while(::FindNextFile(hFind, &fd)); 
        ::FindClose(hFind); 
    } 
}

void create_dir(string path) {
    CreateDirectory(path.c_str(), NULL);
}

#endif