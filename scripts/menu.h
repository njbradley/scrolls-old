// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <chrono>
#include <thread>
using std::stringstream;
using std::vector;
using std::string;
#include <fstream>
using std::ifstream;

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "win.h"
#include "ui.h"

int last_num_ui_verts_menu;

vector<string> worlds;

void render_ui(RenderVecs* vecs, string header, vector<string> & options) {
    int i = 0;
    draw_text(vecs, header, -1, 0.9f);
    for (string name : options) {
        draw_text(vecs, name, -0.5f, 0.5f - i*0.1f);
        i++;
    }
}

void make_ui_buffer(GLuint vertexbuffer, GLuint uvbuffer, GLuint matbuffer, int * num_tris, string header, vector<string> & options) {
	RenderVecs vecs;
	if (last_num_ui_verts_menu != 0) {
		vecs.verts.reserve(last_num_ui_verts_menu*3);
		vecs.uvs.reserve(last_num_ui_verts_menu*2);
		vecs.mats.reserve(last_num_ui_verts_menu);
	}
	render_ui(&vecs, header, options);
	int num_verts = vecs.num_verts;
	last_num_ui_verts_menu = num_verts;
	
	*num_tris = num_verts/3;
	
	GLfloat i = 0.0f;
	GLint j = 0;
	//cout << sizeof(i) << ' ' << sizeof(j) << endl;
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, num_verts*sizeof(i)*3, &vecs.verts.front(), GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, num_verts*sizeof(i)*2, &vecs.uvs.front(), GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, matbuffer);
	glBufferData(GL_ARRAY_BUFFER, num_verts*sizeof(j), &vecs.mats.front(), GL_STATIC_DRAW);
}

class Menu { public:
    virtual void render(GLFWwindow* window, World* world, Player* player, RenderVecs* uivecs) = 0;
};

class Select : public Menu { public:
    string header;
    vector<string> options;
    string chosen;
    function<void(string)> after;
    
    Select(string head, vector<string> & opts, function<void(string)> after_func): header(head), after(after_func) {
        options.swap(opts);
        chosen = "";
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    
    void render(GLFWwindow* window, World* world, Player* player, RenderVecs* uivecs) {
        int i = 0;
        draw_image(uivecs, "popup.bmp", -0.75f, 0.75f, 1.5f, 1.5f);
        draw_image(uivecs, "0popup7.bmp", -0.5f, 0, 1, 0.1f*aspect_ratio);
        draw_text(uivecs, header, -0.75, 0.75f);
        for (string name : options) {
            draw_text(uivecs, name, -0.5f, 0.5f - i*0.1f);
            i++;
        }
        
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            cout << "mouse" << endl;
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
            int index = (int)((double)ypos/screen_y*20) - 4;
            if (index >= 0 and index < worlds.size()) {
                chosen = options[index];
                cout << chosen << endl;
            }
		}
        
        if (chosen != "") {
            after(chosen);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
};
/*
string menu( GLFWwindow* window, GLuint vertexbuffer, GLuint uvbuffer, GLuint matbuffer, GLuint uiVertexArrayID, GLuint uiTextureID, GLuint ui_textures[], int num_uis, GLuint uiProgram, string header, vector<string> & options) {
    bool inmenu = true;
    int num_tris;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    make_ui_buffer(vertexbuffer, uvbuffer, matbuffer, &num_tris, header, options);
    string chosen = "";
    while (inmenu) {
		std::this_thread::sleep_for(std::chrono::milliseconds((int)(20)));
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		
		glBindVertexArray(uiVertexArrayID);
		//*
		glUseProgram(uiProgram);
		
		int uv_ids[num_uis];
		for (int i = 0; i < num_uis; i ++) {
			uv_ids[i] = i;
			glActiveTexture(GL_TEXTURE0+i);
			glBindTexture(GL_TEXTURE_2D, ui_textures[i]);
		}
		
		glUniform1iv(uiTextureID, num_uis, uv_ids);
		
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);
		
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, matbuffer);
		glVertexAttribIPointer(
			2,                                // attribute. No particular reason for 2, but must match the layout in the shader.
			1,                                // size : 1
			GL_INT,                         // type
			                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);
		
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, num_tris*3); // 12*3 indices starting at 0 -> 12 triangles

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		
		
		// desable trancparency
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		//
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
        
        inmenu = glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS and glfwWindowShouldClose(window) == 0;
        
        
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    return chosen;
}*/
