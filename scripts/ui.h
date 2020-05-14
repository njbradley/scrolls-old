#ifndef UI
#define UI
#include <iostream>
#include <string>

#include "text.h"

using std::string;
using std::cout;
using std::endl;

map<string,int> ui_names;

int screen_x = 1600;
int screen_y = 900;

float aspect_ratio = (float)screen_x/screen_y;

void set_screen_dims(int x, int y) {
	screen_x = x;
	screen_y = y;
	aspect_ratio = (float)screen_x/screen_y;
}

void render_debug(MemVecs * vecs, string message) {
	draw_text(vecs, message, -1, 1-pix_y);
	draw_text(vecs, "X", -pix_x/2, -pix_y/2);
}



void draw_image_uv(MemVecs * vecs, string texture, float x, float y, float x_scale, float y_scale, float xuv_start, float xuv_end) {
    float layer = 0;
	int tex_index = ui_names[texture];
	GLfloat face[] = {
		x, y, layer,
		x + x_scale, y, layer,
		x + x_scale, y + y_scale, layer,
		x + x_scale, y + y_scale, layer,
		x, y + y_scale, layer,
		x, y, layer
	};
	GLfloat face_uv[] {
		xuv_start, 1,
		xuv_end, 1,
		xuv_end, 0,
		xuv_end, 0,
		xuv_start, 0,
		xuv_start, 1
	};
    vecs->verts.insert(vecs->verts.end(), begin(face), end(face));
    vecs->uvs.insert(vecs->uvs.end(), begin(face_uv), end(face_uv));
    for (int i = 0; i < 6; i ++) {
        vecs->mats.push_back(tex_index);
    }
    vecs->num_verts += 6;
}

void draw_image(MemVecs * vecs, string texture, float x, float y, float x_scale, float y_scale) {
	draw_image_uv(vecs, texture, x, y, x_scale, y_scale, 0, 1);
}

void draw_icon(MemVecs* vecs, int index, float x, float y, float x_scale = 0.1f, float y_scale = 0.1f*aspect_ratio) {
	draw_image_uv(vecs, "icons.bmp", x, y, x_scale, y_scale, index/32.0f, (index+1)/32.0f);
}

#endif
