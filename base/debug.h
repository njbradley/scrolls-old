#ifndef BASE_DEBUG_H
#define BASE_DEBUG_H

#include "classes.h"
#include "plugins.h"

class Debugger { public:
	PLUGIN_HEAD(Debugger, ());
	
	Debugger() {};
	virtual ~Debugger() {};
	virtual void setup_backtrace() {};
};

class Logger { public:
	PLUGIN_HEAD(Logger, ());
	
	int sel_page = 0;
	
	virtual ~Logger() {};
	virtual ostream& log(int page) = 0;
	virtual ostream& operator[](int page) = 0;
	virtual void render(UIVecs* vecs) = 0;
};

class UILogger : public Logger { public:
	
	class Page : public std::streambuf { public:
		static const int numlines = 30;
		using char_type = std::streambuf::char_type;
		using int_type = std::streambuf::int_type;
		
		vector<vector<char_type>> lines;
		ostream ostr;
		
		Page();
		int_type overflow(int_type lett);
		
		operator ostream& ();
		void render(UIVecs* vecs);
	};
	
	vector<Page> pages;
	
	UILogger();
	virtual ~UILogger() {};
	virtual ostream& log(int page);
	virtual ostream& operator[](int page);
	virtual void render(UIVecs* vecs);
};

extern Plugin<Debugger> debugger;
extern Plugin<Logger> logger;

#endif
