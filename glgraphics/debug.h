#ifndef GL_DEBUG_H
#define GL_DEBUG_H

#include "classes.h"
#include "scrolls/debug.h"

#include <mutex>

class GLDebugLines : public DebugLines { public:
	PLUGIN_HEAD(GLDebugLines);
	
	std::mutex lock;
	
	vector<GLfloat> data;
	vector<string> tags;
	
	GLuint vertexid;
	GLuint program;
	GLuint buffer;
	GLuint mvp_uniform;
	
	GLDebugLines();
	~GLDebugLines();
	virtual void render(vec3 start, vec3 end, vec3 color = vec3(1,1,1), string tag = "");
	virtual void clear();
	virtual void clear(string tag);
	void draw_call(glm::mat4 MVP);
};


#endif
