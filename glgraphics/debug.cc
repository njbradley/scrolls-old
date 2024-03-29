#include "debug.h"
#include "graphics.h"
#include "shader.h"

GLDebugLines::GLDebugLines() {
	glGenVertexArrays(1, &vertexid);
	
	program = LoadShaders(
		PathLib::find_path("shaders/lines.vs").c_str(),
		PathLib::find_path("shaders/lines.fs").c_str()
	);
	
	mvp_uniform = glGetUniformLocation(program, "MVP");
	
	glBindVertexArray(vertexid);
	glGenBuffers(1, &buffer);
}

GLDebugLines::~GLDebugLines() {
	glBindVertexArray(vertexid);
	glDeleteBuffers(1, &buffer);
	
	glDeleteProgram(program);
	glDeleteVertexArrays(1, &vertexid);
}

void GLDebugLines::render(vec3 point1, vec3 point2, vec3 color) {
	float newdata[] = {
		point1.x, point1.y, point1.z,
		color.x, color.y, color.z,
		point2.x, point2.y, point2.z,
		color.x, color.y, color.z,
	};
	
	data.insert(data.end(), newdata, newdata + 12);
}

void GLDebugLines::clear() {
	data.clear();
}

void GLDebugLines::draw_call(glm::mat4 MVP) {
	
	glBindVertexArray(vertexid);
	glUseProgram(program);
	
	glUniformMatrix4fv(mvp_uniform, 1, GL_FALSE, &MVP[0][0]);
	
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), &data[0], GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*6, (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*6, (void*) (sizeof(GLfloat)*3));
	
	glDrawArrays(GL_LINES, 0, data.size()/2);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

EXPORT_PLUGIN(GLDebugLines);
