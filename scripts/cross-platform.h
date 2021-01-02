#ifndef CROSS_PLATFORM_PREDEF
#define CROSS_PLATFORM_PREDEF

#include "classes.h"

void get_files_folder(string folder, vector<string> * names, string expr);
void get_files_folder(string folder, vector<string> * names);
void create_dir(string path);
void delete_dir(string path);
void setup_backtrace();

#endif
