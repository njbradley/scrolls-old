#ifndef UI_PREDEF
#define UI_PREDEF

#include "classes.h"

#include <map>
using std::map;

extern map<string,int> ui_names;

extern int screen_x;
extern int screen_y;

extern float aspect_ratio;

void set_screen_dims(int x, int y);

void render_debug(MemVecs * vecs, string message);

void draw_image_uv(MemVecs * vecs, string texture, float x, float y, float x_scale, float y_scale, float xuv_start, float xuv_end, float yuv_start = 0.0f, float yuv_end = 1.0f);

void draw_image(MemVecs * vecs, string texture, float x, float y, float x_scale, float y_scale);

void draw_icon(MemVecs* vecs, int index, float x, float y, float x_scale = 0.1f, float y_scale = 0.1f*aspect_ratio);

#endif
