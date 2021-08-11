#ifndef BASE_UI_H
#define BASE_UI_H

#include "classes.h"

// A simple UI class, represents a rectangle on
// the screen with an image (defined by the index)
class UIRect { public:
	int id;
	vec2 position;
	vec2 size;
	vec2 uvpos;
	vec2 uvsize;
	
	UIRect(int nid, vec2 pos = vec2(0,0), vec2 nsize = vec2(1,1), vec2 nuvpos = vec2(0,0), vec2 nuvsize = vec2(1,1)):
	id(nid), position(pos), size(nsize), uvpos(nuvpos), uvsize(nuvsize) { }
	UIRect(string name, vec2 pos = vec2(0,0), vec2 nsize = vec2(1,1), vec2 nuvpos = vec2(0,0), vec2 nuvsize = vec2(1,1));
	UIRect(): id(0) {};
};


// A class that represents any UI object displayed on the screen.
// The only requirement is that it can be converted into Rects for
// rendering
class UIObj { public:
	// Renders the object, adding all the rects into the vecs.
	virtual void render(UIVecs* vecs) = 0;
};


class TextLine : public UIObj { public:
	vec2 startpos;
	string text;
	float fontsize;
	
	TextLine(string newtext, vec2 pos, float size);
	virtual void render(UIVecs* vecs);
};

#endif
