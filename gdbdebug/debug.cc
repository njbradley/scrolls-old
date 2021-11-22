#include "debug.h"
#include <time.h>
#include <sys/stat.h>
#include <thread>

const char gdb_command[] = "gdb %s %i "
" -ex \"set pagination off\""
" -ex \"set logging file crash-report.txt\""
" -ex \"set logging redirect on\""
" -ex \"set logging on\""

" -ex \"echo \\n------ THREADS --------\\n\\n\""
" -ex \"info threads\""

" -ex \"echo \\n------ THREADS BACKTRACE --------\\n\\n\""
" -ex \"thread apply all bt\""

" -ex \"echo \\n------ FULL THREADS BACKTRACE --------\\n\\n\""
" -ex \"thread apply all bt full\""

" -ex \"set logging off\""
" -ex \"set logging redirect off\""
" -ex \"set pagination on\""
" -ex \"set confirm off\"";

char formatted_gdb_command[sizeof(gdb_command) + 300];
char log_filename[100];

void make_log_filename(char log_filename[]) {
	time_t timer;
  struct tm* tm_info;
	
  timer = time(NULL);
  tm_info = localtime(&timer);

  strftime(log_filename, 100, "crash-reports/%Y-%m-%d_%H-%M-%S.txt", tm_info);
}

void cleanup(int sig) {
	rename("crash-report.txt", log_filename);
	cout << "EXITING " << endl;
	signal (sig, SIG_DFL);
  raise (sig);
}

#ifdef _WIN32

#include <processthreadsapi.h>
#include <libloaderapi.h>

void format_command() {
	char path[100];
	GetModuleFileName(NULL, path, 100);
	sprintf(formatted_gdb_command, gdb_command, path, GetCurrentProcessId());
}

#else

#include <unistd.h>
#include <stdio.h>

void format_command() {
	char path[100];
	readlink("/proc/self/exe", path, 100)
	sprintf(formatted_gdb_command, gdb_command, path, getpid());
}

#endif

void handler(int sig) {
	cout << "Recieved signal " << sig << " on thread " << std::hex << GetCurrentThreadId() << std::dec << endl;
	system(formatted_gdb_command);
	cleanup(sig);
}

GDBDebugger::GDBDebugger() {
	format_command();
	
	struct stat stat_info;
	if (!stat("crash-reports/", &stat_info)) {
		mkdir("crash-reports/");
	}
	
	make_log_filename(log_filename);
	
	signal(SIGINT, handler);
	signal(SIGABRT, handler);
	signal(SIGSEGV, handler);
}

EXPORT_PLUGIN(GDBDebugger);
