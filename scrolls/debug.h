#ifndef BASE_DEBUG_H
#define BASE_DEBUG_H

#include "classes.h"
#include "plugins.h"
#include <mutex>

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
	virtual void clear(int page) = 0;
	virtual void pause() = 0;
	virtual void unpause() = 0;
	virtual ostream& operator[](int page) = 0;
	virtual void render(UIVecs* vecs) = 0;
};

class UILogger : public Logger { public:
	
	class Page : public std::streambuf { public:
		static const int numlines = 80;
		using char_type = std::streambuf::char_type;
		using int_type = std::streambuf::int_type;
		std::mutex lock;
		bool paused = false;
		
		vector<vector<char_type>> lines;
		ostream ostr;
		
		Page();
		int_type overflow(int_type lett);
		
		operator ostream& ();
		void render(UIVecs* vecs);
		void clear();
	};
	
	vector<Page> pages;
	
	UILogger();
	virtual ~UILogger() {};
	virtual ostream& log(int page);
	virtual void clear(int page);
	virtual void pause();
	virtual void unpause();
	virtual ostream& operator[](int page);
	virtual void render(UIVecs* vecs);
};

class DebugLines { public:
	PLUGIN_HEAD(DebugLines, ());
	
	virtual void render(vec3 start, vec3 end, vec3 color = vec3(1,1,1)) { }
	void render(Hitbox hitbox, vec3 color = vec3(1,1,1));
	virtual void clear() { };
};

extern Plugin<Debugger> debugger;
extern Plugin<Logger> logger;
extern Plugin<DebugLines> debuglines;

#endif
