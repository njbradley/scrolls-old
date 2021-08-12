#include "ui.h"
#include "rendervecs.h"
#include "graphics.h"
#include "libraries.h"

UIRect::UIRect(string name, vec2 pos, vec2 nsize, vec2 nuvpos, vec2 nuvsize):
UIRect(graphics->ui_textures()->getindex(name), pos, nsize, nuvpos, nuvsize) {
	
}



TextLine::TextLine(string newtext, vec2 pos, float size): text(newtext), startpos(pos), fontsize(size) {
	
}

void TextLine::render(UIVecs* vecs) {
	UIRect rect("letters.png", startpos, vec2(fontsize, fontsize*2), vec2(0,0), vec2(1.0f/16, 1.0f/6));
	
	for (char lett : text) {
		if (lett == '\n') {
			rect.position.y += rect.size.y;
		} else if (' ' <= lett and lett <= '~') {
			rect.uvpos = vec2(((lett - ' ') % 16) / 16.0f, ((lett - ' ') / 16) / 6.0f);
			vecs->add(rect);
			rect.position.x += rect.size.x;
		}
	}
}
