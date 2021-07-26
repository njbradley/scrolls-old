#ifndef TEXT_PREDEF
#define TEXT_PREDEF

#include "classes.h"

extern const float pix_y;
extern const float pix_x;


void textline_draw(MemVecs* vecs, string str, float x, float y, int size);

float textline_getlength(string str, int size);

void draw_text(MemVecs* vecs, string str, float x, float y, int size, string alignment);

bool collide_last_text(float x, float y);

void draw_text(MemVecs* vecs, string str, float x, float y, int size);

void draw_text(MemVecs* vecs, string str, float x, float y, string alignment);

void draw_text(MemVecs* vecs, string str, float x, float y);

#endif
