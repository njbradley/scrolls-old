#include "debug.h"

#include "ui.h"
#include "rendervecs.h"
#include "physics.h"

DEFINE_PLUGIN(Logger);
DEFINE_PLUGIN(Debugger);
EXPORT_PLUGIN(Debugger);
DEFINE_AND_EXPORT_PLUGIN(DebugLines);


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

void UILogger::Page::clear() {
	for (vector<char_type>& line : lines) {
		line.clear();
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

void UILogger::clear(int page) {
	if (page >= 3) {
		pages[page-3].clear();
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



void DebugLines::render(Hitbox hitbox, vec3 color) {
	vec3 points[8];
	hitbox.points(points);
	// x=0 face
	render(points[0], (points[0]+points[1])/2.0f, vec3(0,0,1));
	render((points[0]+points[1])/2.0f, points[1], color);
	render(points[0], (points[0]+points[2])/2.0f, vec3(0,1,0));
	render((points[0]+points[2])/2.0f, points[2], color);
	render(points[2], points[3], color);
	render(points[1], points[3], color);
	// x=1 face
	render(points[4], points[5], color);
	render(points[4], points[6], color);
	render(points[6], points[7], color);
	render(points[5], points[7], color);
	// connectors
	render(points[0], (points[0]+points[4])/2.0f, vec3(1,0,0));
	render((points[0]+points[4])/2.0f, points[4], color);
	render(points[1], points[5], color);
	render(points[2], points[6], color);
	render(points[3], points[7], color);
}



Plugin<Debugger> debugger;
Plugin<Logger> logger;
Plugin<DebugLines> debuglines;

EXPORT_PLUGIN(UILogger);
