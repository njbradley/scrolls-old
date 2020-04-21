#ifndef WIN
#define WIN
#include <Windows.h>
#include <direct.h>
//#include <windows.h>
using std::string;

void get_files_folder(string folder, vector<string> * names, string expr)
{
    string search_path = folder + "/" + expr;
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

void get_files_folder(string folder, vector<string>* names) {
    get_files_folder(folder, names, "*.*");
}

void create_dir(string path) {
    CreateDirectory(path.c_str(), NULL);
}

void delete_dir(string path) {
  _rmdir(path.c_str());
}




#endif
