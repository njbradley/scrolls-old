#ifndef TEXTH
#define TEXTH

// Include GLEW
#include <GL/glew.h>
#include <iostream>

// Include GLFW
#include <GLFW/glfw3.h>
#include <map>

#include "blocks-predef.h"
#include "rendervec.h"

using std::string;
using std::cout;
using std::endl;
using std::begin;
using std::end;

const char letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*()-=[]\\;',./_+{}|:\"<> ?";
const float pix_x = 5/200.0f;
const float pix_y = pix_x*2;

float bounds[4] = {0, 0, 0, 0}; //min x, max x, min y, max y


void textline_draw(MemVecs* vecs, string str, float x, float y, int size) {
    float original_x = x;
    for (char c : str) {
        int index = 92;
        for (int i = 0; i < 93; i ++) {
            if (c == letters[i]) {
                index = i;
                break;
            }
        }
        
        GLfloat face[] = {
            x, y, 0,
            x + pix_x*size, y, 0,
            x + pix_x*size, y + pix_y*size, 0,
            x + pix_x*size, y + pix_y*size, 0,
            x, y + pix_y*size, 0,
            x, y, 0
        };
        float uvpos = index / 93.0;
        GLfloat face_uv[] {
            uvpos, 1,
            uvpos + 1/93.0f, 1,
            uvpos + 1/93.0f, 0,
            uvpos + 1/93.0f, 0,
            uvpos, 0,
            uvpos, 1
        };
        
        vecs->verts.insert(vecs->verts.end(), begin(face), end(face));
        vecs->uvs.insert(vecs->uvs.end(), begin(face_uv), end(face_uv));
        for (int i = 0; i < 6; i ++) {
            vecs->mats.push_back(0);
        }
        vecs->num_verts += 6;
        x += pix_x*size;
    }
}

float textline_getlength(string str, int size) {
    //very simple currently because it is a fixed width font
    return pix_x * size * str.length();
}

void draw_text(MemVecs* vecs, string str, float x, float y, int size, string alignment) {
    //split the text string into a vector of lines
    vector<string> lines;
    lines.push_back("");
    for (char c: str) {
        if (c == '\n') {
            lines.push_back("");
        }
        else {
            lines[lines.size()-1] += c;
        }
    }

    //find the widest width of the text chunk
    //used for bounding box
    float width = 0.0f;
    for (string line: lines) {
        width = std::max(width, textline_getlength(line, size));
    }

    float linex;
    float min_x = 1.0f;
    float liney = y;
    for (string line: lines) {
        float linewidth = textline_getlength(line, size);
        
        if (alignment == "right") {
            linex = x - linewidth;
        }
        else if (alignment == "center") {
            linex = x - linewidth / 2;
        }
        else {
            linex = x;
        }

        min_x = std::min(min_x, linex);

        textline_draw(vecs, line, linex, liney, size);

        liney -= pix_y * size;
    }

    //min x, max x, min y, max y
    bounds[0] = min_x;
    bounds[1] = min_x + width;
    bounds[2] = liney + pix_y * size;
    bounds[3] = y + pix_y * size;
}

bool collide_last_text(float x, float y) {
    return x > bounds[0] && x < bounds[1] && y > bounds[2] && y < bounds[3];
}

draw_text(MemVecs* vecs, string str, float x, float y, int size) {
    draw_text(vecs, str, x, y, size, "left");
}

draw_text(MemVecs* vecs, string str, float x, float y, string alignment) {
    draw_text(vecs, str, x, y, 1, alignment);
}

draw_text(MemVecs* vecs, string str, float x, float y) {
    draw_text(vecs, str, x, y, 1, "left");
}

#endif
