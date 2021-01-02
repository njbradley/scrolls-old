#ifndef CROSS_PLATFORM
#define CROSS_PLATFORM

#include "cross-platform.h"

#ifdef _WIN32

#include <Windows.h>
#include <direct.h>
#include <chrono>
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
            if(! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
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






#include <dwarfstack.h>

#include <stdio.h>
#include <windows.h>

void crash_header(ostream& out) {
  out << "Scrolls crash log" << endl;
  //out << "Application crashed at " << std::chrono::system_clock::now() << endl;
}

template <ostream& out>
static void stderrPrint(
    uint64_t addr,const char *filename,int lineno,const char *funcname,
    void *context,int columnno ) {
  int *count = (int*)context;
  
  const char *delim = strrchr( filename,'/' );
  if( delim ) filename = delim + 1;
  delim = strrchr( filename,'\\' );
  if( delim ) filename = delim + 1;

  void *ptr = (void*)(uintptr_t)addr;
  if (lineno == DWST_BASE_ADDR) {
    out << "base address 0x" << std::hex << ptr << std::dec << " (" << filename << ")" << endl;
  } else if (lineno == DWST_NOT_FOUND or lineno == DWST_NO_DBG_SYM or lineno == DWST_NO_SRC_FILE) {
    out << "    stack " << (*count)++ << " 0x" << std::hex << ptr << std::dec << " (" << filename << ")" << endl;
  } else {
    if( ptr ) {
      out << "    stack " << (*count)++ << " 0x" << std::hex << ptr << std::dec;
    } else {
      out << "                " << (int)sizeof(void*)*2 << endl;
    }
    out << " (" << filename << ':' << lineno;
    if( columnno>0 ) {
      out << ':' << columnno;
    }
    out << ')';
    if( funcname ) {
      out << " [" << funcname << ']';
    }
    out << endl;
  }
}

template <ostream& out>
LONG exceptionPrinter( LPEXCEPTION_POINTERS ep ) {
  
  DWORD code = ep->ExceptionRecord->ExceptionCode;
  const char *desc = "";
  switch( code ) {
#define EX_DESC( name ) \
    case EXCEPTION_##name: desc = " (" #name ")"; \
                           break

    EX_DESC( ACCESS_VIOLATION );
    EX_DESC( ARRAY_BOUNDS_EXCEEDED );
    EX_DESC( BREAKPOINT );
    EX_DESC( DATATYPE_MISALIGNMENT );
    EX_DESC( FLT_DENORMAL_OPERAND );
    EX_DESC( FLT_DIVIDE_BY_ZERO );
    EX_DESC( FLT_INEXACT_RESULT );
    EX_DESC( FLT_INVALID_OPERATION );
    EX_DESC( FLT_OVERFLOW );
    EX_DESC( FLT_STACK_CHECK );
    EX_DESC( FLT_UNDERFLOW );
    EX_DESC( ILLEGAL_INSTRUCTION );
    EX_DESC( IN_PAGE_ERROR );
    EX_DESC( INT_DIVIDE_BY_ZERO );
    EX_DESC( INT_OVERFLOW );
    EX_DESC( INVALID_DISPOSITION );
    EX_DESC( NONCONTINUABLE_EXCEPTION );
    EX_DESC( PRIV_INSTRUCTION );
    EX_DESC( SINGLE_STEP );
    EX_DESC( STACK_OVERFLOW );
  }
  out << endl << "CODE: " << std::hex << code << std::dec << desc << endl;

  if( code==EXCEPTION_ACCESS_VIOLATION &&
  ep->ExceptionRecord->NumberParameters==2 ) {
    ULONG_PTR flag = ep->ExceptionRecord->ExceptionInformation[0];
    ULONG_PTR addr = ep->ExceptionRecord->ExceptionInformation[1];
    out << ((flag==8) ? "data execution prevention" : (flag?"write access":"read access"))
    << " violation at 0x" << std::hex << addr << std::dec << endl;
  }
  
  out << endl << "EXCEPTION BACKTRACE" << endl;
  int count = 0;
  dwstOfException( ep->ContextRecord,&stderrPrint<out>,&count );
  
  out << endl << "FULL BACKTRACE" << endl;
  count = 0;
  dwstOfLocation(stderrPrint<out>, &count);
  
  fflush( stderr );

  return( EXCEPTION_EXECUTE_HANDLER );
}

static LONG WINAPI exceptionHandler( LPEXCEPTION_POINTERS ep ) {
  LONG result = exceptionPrinter<std::cerr>(ep);
  return result;
}

void sig_abort(int sig) {
  std::cerr << endl << "SIGNAL: " << sig << " (SIGABRT)" << endl;
  int count = 0;
  dwstOfLocation(stderrPrint<std::cerr>, &count);
  exit(sig);
}

void sig_int(int sig) {
  std::cerr << endl << "SIGNAL: " << sig << " (SIGINT)" << endl;
  int count = 0;
  dwstOfLocation(stderrPrint<std::cerr>, &count);
  exit(sig);
}



void setup_backtrace() {
  #ifdef SCROLLS_DEBUG
  //dwstExceptionDialog( __DATE__ " - " __TIME__ );
	SetUnhandledExceptionFilter( exceptionHandler );
  signal(SIGABRT, sig_abort);
  signal(SIGINT, sig_int);
  #endif
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
        if (dir_name != "." and dir_name != ".." and dir_name != ".DS_Store") {
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

void setup_backtrace() {
  
}

#endif

#endif
