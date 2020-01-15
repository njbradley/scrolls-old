#include <iostream>
#include <string>

#include "text.h"

using std::string;
using std::cout;
using std::endl;

float aspect_ratio = 1024/768;

void render_debug(RenderVecs * vecs, string message) {
	draw_text("debug message: game scrolls\n" + message, -1, 1-pix_y, vecs);
	draw_text("X", -pix_x/2, -pix_y/2, vecs);
}

void draw_image(RenderVecs * vecs, int tex_index, float x, float y, float x_scale, float y_scale) {
    float layer = 0;
	GLfloat face[] = {
		x, y, layer,
		x + x_scale, y, layer,
		x + x_scale, y + y_scale, layer,
		x + x_scale, y + y_scale, layer,
		x, y + y_scale, layer,
		x, y, layer
	};
	GLfloat face_uv[] {
		0, 1,
		1, 1,
		1, 0,
		1, 0,
		0, 0,
		0, 1
	};
    vecs->verts.insert(vecs->verts.end(), begin(face), end(face));
    vecs->uvs.insert(vecs->uvs.end(), begin(face_uv), end(face_uv));
    for (int i = 0; i < 6; i ++) {
        vecs->mats.push_back(tex_index);
    }
    vecs->num_verts += 6;
}