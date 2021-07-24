#include "debug.h"

#ifdef _WIN32

#include <dwarfstack.h>

#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <dbghelp.h>
#include <time.h>
#include <filesystem>

void safe_make_dir(string path) {
  if (!std::filesystem::exists(path)) {
		std::filesystem::create_directory(path);
	}
}

string timestamp() {
  time_t rawtime = time(NULL);
  struct tm * dt;
  char timestr[30];
  dt = localtime(&rawtime);
  strftime(timestr, sizeof(timestr), "%Y-%m-%d_%H-%M-%S", dt);
  return timestr;
}

void crash_header(ostream& out) {
  out << "Scrolls crash log" << endl;
  out << "Crash time: " << timestamp() << endl;
  out << "Compile time: " << __DATE__ << ' ' << __TIME__ << endl;
}

static void stderrPrint(
    uint64_t addr,const char *filename,int lineno,const char *funcname,
    void *context,int columnno ) {
  pair<int*,ostream&> context_data = *(pair<int*,ostream&>*)context;
  int *count = context_data.first;
  ostream& out = context_data.second;
  
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
      out << "                " << (int)sizeof(void*)*2;
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

// csp ... current stack pointer
// cip ... current instruction pointer
// cfp ... current frame pointer
#ifdef _WIN64
#define csp Rsp
#define cip Rip
#define cfp Rbp
#define MACH_TYPE IMAGE_FILE_MACHINE_AMD64
#else
#define csp Esp
#define cip Eip
#define cfp Ebp
#define MACH_TYPE IMAGE_FILE_MACHINE_I386
#endif

#define MAX_FRAMES 32

int captureStackWalk(HANDLE process, HANDLE thread, CONTEXT *context, uintptr_t *frames, int size) {
  CONTEXT contextCopy;
  memcpy( &contextCopy,context,sizeof(CONTEXT) );
  context = &contextCopy;

  STACKFRAME64 stack;
  ZeroMemory( &stack,sizeof(STACKFRAME64) );
  stack.AddrPC.Offset = context->cip;
  stack.AddrPC.Mode = AddrModeFlat;
  stack.AddrStack.Offset = context->csp;
  stack.AddrStack.Mode = AddrModeFlat;
  stack.AddrFrame.Offset = context->cfp;
  stack.AddrFrame.Mode = AddrModeFlat;

  int count = 0;
  
  while( StackWalk64(MACH_TYPE,process,thread,&stack,context,
        NULL,SymFunctionTableAccess64,SymGetModuleBase64,NULL) &&
      count<size )
  {
    uintptr_t frame = stack.AddrPC.Offset;
    if( !frame ) break;
    
    if( count ) frame--;
    frames[count++] = frame;
  }
  
  return( count );
}

int customOfException(HANDLE thread, CONTEXT *contextP, dwstCallback *callbackFunc, void *callbackContext )
{
  uintptr_t frames[MAX_FRAMES];
  int count = 0;
  
  HANDLE process = GetCurrentProcess();

  SymSetOptions( SYMOPT_LOAD_LINES );
  SymInitialize( process,NULL,TRUE );

  count += captureStackWalk(process, thread, contextP,frames+count,MAX_FRAMES-count);

  count = dwstOfProcess( frames,count, callbackFunc,callbackContext );

  SymCleanup( process );
  return( count );
}



LONG exceptionPrinter(ostream& out, LPEXCEPTION_POINTERS ep ) {
  
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
  pair<int*,ostream&> context_data(&count, out);
  customOfException(GetCurrentThread(), ep->ContextRecord,&stderrPrint,&context_data );
  
  out << endl << "FULL BACKTRACE" << endl;
  count = 0;
  dwstOfLocation(&stderrPrint, &context_data);
  
  fflush( stderr );

  return( EXCEPTION_EXECUTE_HANDLER );
}

void printError(const TCHAR* msg )
{
  DWORD eNum;
  TCHAR sysMsg[256];
  TCHAR* p;

  eNum = GetLastError( );
  FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
         NULL, eNum,
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
         sysMsg, 256, NULL );

  // Trim the end of the line and terminate it with a null
  p = sysMsg;
  while( ( *p > 31 ) || ( *p == 9 ) )
    ++p;
  do { *p-- = 0; } while( ( p >= sysMsg ) &&
                          ( ( *p == '.' ) || ( *p < 33 ) ) );

  // Display the message
  _tprintf( "\n  WARNING: %s failed with error %d (%s)", msg, eNum, sysMsg );
}


BOOL iter_all_threads(ostream& out, DWORD dwOwnerPID, void (*callback)(ostream& out, HANDLE thread))
{
  HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
  THREADENTRY32 te32;
 
  // Take a snapshot of all running threads
  hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 );
  if( hThreadSnap == INVALID_HANDLE_VALUE ) {
    return( FALSE );
  }
  te32.dwSize = sizeof(THREADENTRY32 );
  
  if( !Thread32First( hThreadSnap, &te32 ) )
  {
    printError( "Thread32First" );
    CloseHandle( hThreadSnap );
    return( FALSE );
  }
  
  do
  {
    if( te32.th32OwnerProcessID == dwOwnerPID )
    {
      out << endl << "Thread id: 0x" << std::hex << te32.th32ThreadID << 1010 << endl;
			HANDLE thread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
			if (thread == nullptr) {
				printError( "OpenThread");
				CloseHandle(thread);
				return FALSE;
			}
			callback(out, thread);
			CloseHandle(thread);
    }
  } while( Thread32Next(hThreadSnap, &te32 ) );
  
  CloseHandle( hThreadSnap );
  return( TRUE );
}

void get_thread_trace(ostream& out, HANDLE thread) {
  CONTEXT context;
  memset(&context, 0, sizeof(CONTEXT));
  context.ContextFlags = CONTEXT_FULL;
  if (!GetThreadContext(thread, &context)) {
    printError("GetThreadContext");
    return;
  }
  
  int count = 0;
  pair<int*,ostream&> callback_data(&count, out);
  
  customOfException(thread, &context,&stderrPrint,&callback_data);
}

void get_all_threads_trace(ostream& out) {
  iter_all_threads(out, GetCurrentProcessId(), get_thread_trace);
}

static LONG WINAPI exceptionHandler( LPEXCEPTION_POINTERS ep ) {
  std::cerr << "SIGNAL: UNHANDLED " << endl;
  safe_make_dir("crash-reports");
  ofstream ofile("crash-reports/" + timestamp() + ".txt");
  crash_header(ofile);
  stringstream sigtrace_str;
  
  LONG result = exceptionPrinter(sigtrace_str, ep);
  
  std::cerr << sigtrace_str.str();
  ofile << sigtrace_str.str();
  
  ofile << endl << "ALL THREADS:" << endl;
  get_all_threads_trace(ofile);
  return result;
}

void sig_abort(int sig) {
  std::cerr << endl << "SIGNAL: " << sig << " (SIGABRT)" << endl;
  safe_make_dir("crash-reports");
  ofstream ofile("crash-reports/" + timestamp() + ".txt");
  crash_header(ofile);
  stringstream sigtrace_str;
  
  ofile << endl << "SIGNAL: " << sig << " (SIGABRT)" << endl;
  
  int count = 0;
  pair<int*,ostream&> context_data(&count,sigtrace_str);
  dwstOfLocation(stderrPrint, &context_data);
  
  std::cerr << sigtrace_str.str();
  ofile << sigtrace_str.str();
  
  ofile << endl << "ALL THREADS:" << endl;
  get_all_threads_trace(ofile);
  exit(sig);
}

void sig_int(int sig) {
  std::cerr << endl << "SIGNAL: " << sig << " (SIGINT)" << endl;
  safe_make_dir("crash-reports");
  ofstream ofile("crash-reports/" + timestamp() + ".txt");
  crash_header(ofile);
  stringstream sigtrace_str;
  ofile << endl << "SIGNAL: " << sig << " (SIGINT)" << endl;
  
  get_all_threads_trace(ofile);
  
  std::cerr << sigtrace_str.str();
  ofile << sigtrace_str.str();
  
  exit(sig);
}



void DwarfDebugger::setup_backtrace() {
  #ifdef SCROLLS_DEBUG
  //dwstExceptionDialog( __DATE__ " - " __TIME__ );
	SetUnhandledExceptionFilter( exceptionHandler );
  signal(SIGABRT, sig_abort);
  signal(SIGINT, sig_int);
  #endif
}

#else

void DwarfDebugger::setup_backtrace() {
	
}

#endif

EXPORT_PLUGIN(DwarfDebugger);
