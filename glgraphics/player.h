#ifndef GL_PLAYER_H
#define GL_PLAYER_H

#include "classes.h"
#include "scrolls/player.h"

class GLControls : public Controls { public:
	
	GLControls();
	
	virtual bool key_pressed(int keycode);
	virtual bool mouse_button(int button);
	virtual ivec2 mouse_pos();
	virtual void mouse_pos(ivec2 newpos);
	virtual int scroll_rel();
};

#endif
