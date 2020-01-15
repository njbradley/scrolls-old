// Include GLEW
#include <GL/glew.h>
#include <iostream>

// Include GLFW
#include <GLFW/glfw3.h>
#include <map>

#include "blocks.h"

using std::string;
using std::cout;
using std::endl;
using std::begin;
using std::end;



const char letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*()-=[]\\;',./_+{}|:\"<> ?";
const float pix_x = 5/200.0f;
const float pix_y = pix_x*2;

void draw_text(string str, float x, float y, RenderVecs* vecs) {
    float original_x = x;
    for (char c : str) {
        if (c == '\n') {
            x = original_x;
            y -= pix_y;
            continue;
        }
        int index = -1;
        for (int i = 0; i < 93; i ++) {
            if (c == letters[i]) {
                index = i;
                break;
            }
        }
        if (index == -1) {
            index = 92;
        }
        
        GLfloat face[] = {
            x, y, 0,
            x + pix_x, y, 0,
            x + pix_x, y + pix_y, 0,
            x + pix_x, y + pix_y, 0,
            x, y + pix_y, 0,
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
        x += pix_x;
    }
}

