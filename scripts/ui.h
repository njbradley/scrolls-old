#include <iostream>
#include <string>

#include "text.h"

using std::string;
using std::cout;
using std::endl;

map<string,int> ui_names;

float aspect_ratio = 1024/768;

void render_debug(RenderVecs * vecs, string message) {
	draw_text("-----debug message: game scrolls-----\n" + message, -1, 1-pix_y, vecs);
	draw_text("X", -pix_x/2, -pix_y/2, vecs);
}



void draw_image_uv(RenderVecs * vecs, string texture, float x, float y, float x_scale, float y_scale, float xuv_start, float xuv_end) {
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

void draw_image(RenderVecs * vecs, string texture, float x, float y, float x_scale, float y_scale) {
	draw_image_uv(vecs, texture, x, y, x_scale, y_scale, 0, 1);
}
