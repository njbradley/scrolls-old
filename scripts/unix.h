#ifndef UNIX
#define UNIX

#include <sys/types.h>
#include <dirent.h>

#include <vector>
#include <string>
#include <iostream>

#include <stdio.h> 

#include <sys/stat.h>

using std::string;
using std::vector;
using std::cout;
using std::endl;



//void read_directory(const std::string& name, stringvec& v)
void get_files_folder(string folder, vector<string> * names)
{
    DIR* dirp = opendir(folder.c_str());
    struct dirent * dp;
    while ((dp = readdir(dirp)) != NULL) {
	string dir_name(dp->d_name);
	if (dir_name != "." and dir_name != "..") {
        	names->push_back(dp->d_name);
	}
    }
    closedir(dirp);
}

void create_dir(string path) {
    mkdir(path.c_str(), 0775);
}




#endif
