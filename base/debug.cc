#include "debug.h"

DEFINE_PLUGIN(Logger);
DEFINE_PLUGIN(Debugger);


ostream& Logger::log(int page) {
	cout << "[Page " << page << "]: ";
	return cout;
}

ostream& Logger::operator[] (int page) {
	return log(page);
}

Plugin<Debugger> debugger;
Plugin<Logger> logger;
