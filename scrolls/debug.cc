#include "debug.h"

#include "ui.h"
#include "rendervecs.h"
#include "player.h"

DEFINE_PLUGIN(Logger);
DEFINE_PLUGIN(Debugger);



UILogger::Page::Page(): lines(60), ostr(this) {
	
}

UILogger::Page::int_type UILogger::Page::overflow(UILogger::Page::int_type lett) {
	if (!paused) {
		std::lock_guard<std::mutex> guard(lock);
		if (lett == '\n') {
			lines.erase(lines.begin());
			lines.emplace_back();
		} else {
			lines.back().push_back(lett);
		}
	}
	return lett;
}

UILogger::Page::operator ostream& () {
	return ostr;
}

void UILogger::Page::render(UIVecs* vecs) {
	static const float size = 2.0f / (lines.size());
	vec2 pos (-1, 1-size);
	
	std::lock_guard<std::mutex> guard(lock);
	for (vector<char_type>& line : lines) {
		for (char_type let : line) {
			vecs->add(UIChar( let, pos, size/2));
			pos.x += size/2;
		}
		pos.x = -1;
		pos.y -= size;
	}
}


UILogger::UILogger(): pages(10) {
	
}

ostream& UILogger::log(int page) {
	if (page == 0) {
		cout << "[INFO]: ";
		return cout;
	} else if (page == 1) {
		cout << "[LOG]: ";
		return cout;
	} else if (page == 2) {
		std::cerr << "[ERR]: ";
		return std::cerr;
	} else {
		return pages[page-3];
	}
}

ostream& UILogger::operator[] (int page) {
	return log(page);
}

void UILogger::pause() {
	if (!pages[sel_page-3].paused) {
		log(sel_page) << "---------- Paused (" << getTime() << ") ----------";
		pages[sel_page-3].paused = true;
	}
}

void UILogger::unpause() {
	if (pages[sel_page-3].paused) {
		pages[sel_page-3].paused = false;
		log(sel_page) << endl << "---------- Resumed (" << getTime() << ") ----------" << endl;
	}
}

void UILogger::render(UIVecs* vecs) {
	if (sel_page >= 3) {
		pages[sel_page-3].render(vecs);
	}
}

Plugin<Debugger> debugger;
Plugin<Logger> logger;

EXPORT_PLUGIN(UILogger);
