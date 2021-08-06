#include "ui.h"


TextLine::TextLine(string newtext, vec2 pos, float size): text(newtext), index(0), curpos(pos) {
	rect.id = graphics->ui_textures()->getindex("letters.png");
	rect.position = pos;
	rect.size = vec2(size, size*2);
	rect.uvsize = vec2(1.0f/16, 1.0f/6);
}

bool TextLine::to_rects(UIRect* nextrect) {
	char lett;
	while ((lett = text[index++]) == '\n') {
		rect.position.y += rect.size.y;
	}
	
	*nextrect = rect;
	nextrect->uvpos = vec2(((lett - ' ') % 16) / 16.0f, ((lett - ' ') / 16) / 6.0f)
	
	rect.position.x += rect.size.x;
	
	return index >= text.size();
}
