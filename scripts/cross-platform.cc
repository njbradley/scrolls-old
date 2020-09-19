#ifndef CROSS_PLATFORM
#define CROSS_PLATFORM

#include "cross-platform.h"

#ifdef _WIN32

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



#else

#include <sys/types.h>
#include <dirent.h>

#include <vector>
#include <string>
#include <iostream>

#include <stdio.h>
#include <set>
#include <sys/stat.h>

using std::string;
using std::vector;
using std::cout;
using std::endl;

//void read_directory(const std::string& name, stringvec& v)
void get_files_folder(string folder, vector<string> * names)
{
    std::set<string> filenames;
    DIR* dirp = opendir(folder.c_str());
    struct dirent * dp;
    while ((dp = readdir(dirp)) != NULL) {
        string dir_name(dp->d_name);
        if (dir_name != "." and dir_name != "..") {
            filenames.insert(dp->d_name);
        }
    }
    closedir(dirp);
    for (string name : filenames) {
        names->push_back(name);
        //cout << name << ", ";
    }
    //cout << endl;
    //exit(1);
}

void create_dir(string path) {
    mkdir(path.c_str(), 0775);
}

#endif

#endif
