#ifndef GL_DEBUG_H
#define GL_DEBUG_H

#include "classes.h"
#include "scrolls/debug.h"

class GLDebugLines : public DebugLines { public:
	vector<GLfloat> data;
	
	GLuint vertexid;
	GLuint program;
	GLuint buffer;
	GLuint mvp_uniform;
	
	GLDebugLines();
	~GLDebugLines();
	virtual void render(vec3 start, vec3 end);
	virtual void clear();
	void draw_call(glm::mat4 MVP);
};


#endif
