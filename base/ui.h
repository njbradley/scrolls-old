#ifndef BASE_UI_H
#define BASE_UI_H

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
	UIRect(): id(-1) {};
};


// A class that represents any UI object displayed on the screen.
// The only requirement is that it can be converted into Rects for
// rendering
class UIObj { public:
	// Used to reduce a UIObj into UIRects for rendering
	// the return value is whether there are more rects
	// left (for objects that need multiple rects)
	// when converting to rects, it should be looped over until
	// returns false
	virtual bool to_rects(UIRect* rect) = 0;
};

#endif
