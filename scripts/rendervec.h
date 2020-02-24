#ifndef RENDERVEC
#define RENDERVEC
#include <iostream>
#include <GL/glew.h>
using std::cout;
using std::endl;
#include <ctime>
#include "rendervec-predef.h"


void RenderVecs::add_face(GLfloat* newverts, GLfloat* newuvs, GLfloat newlight, GLint mat) {
    verts.insert(verts.end(), newverts, newverts+6*3);
    uvs.insert(uvs.end(), newuvs, newuvs+6*2);
    for (int i = 0; i < 6; i ++) {
        light.push_back(newlight);
        mats.push_back(mat);
    }
    num_verts += 6;
}




void GLVecs::set_buffers(GLuint verts, GLuint uvs, GLuint light, GLuint mats, int start_size) {
    vertexbuffer = verts;
    uvbuffer = uvs;
    lightbuffer = light;
    matbuffer = mats;
    size_alloc = start_size;
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, start_size*3*sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, start_size*2*sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, lightbuffer);
    glBufferData(GL_ARRAY_BUFFER, start_size*sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, matbuffer);
    glBufferData(GL_ARRAY_BUFFER, start_size*sizeof(GLint), NULL, GL_STATIC_DRAW);
    
    
    
    //cout << "err: " << std::hex << glGetError() << std::dec << endl;
}

pair<int,int> GLVecs::add(RenderVecs* newvecs) {
    int old_num_verts = num_verts;
    int location = num_verts;
    
    for (int i = 0; i < empty.size(); i ++) {
        int num_faces = empty[i].second;
        if (num_faces == newvecs->num_verts/6) {
            location = empty[i].first*6;
            pair<int,int> index = empty[i];
            //cout << "used: " << empty[i].first << ' ' << empty[i].second << " for " << newvecs->num_verts << endl;
            empty.erase(empty.begin()+i);
            
            glNamedBufferSubData(vertexbuffer, location*3*sizeof(GLfloat), newvecs->verts.size()*sizeof(GLfloat), &newvecs->verts.front());
            glNamedBufferSubData(uvbuffer, location*2*sizeof(GLfloat), newvecs->uvs.size()*sizeof(GLfloat), &newvecs->uvs.front());
            glNamedBufferSubData(lightbuffer, location*sizeof(GLfloat), newvecs->light.size()*sizeof(GLfloat), &newvecs->light.front());
            glNamedBufferSubData(matbuffer, location*sizeof(GLint), newvecs->mats.size()*sizeof(GLfloat), &newvecs->mats.front());
            
            return index;
        }
    }
    if (num_verts+newvecs->light.size() > size_alloc) {
        cout << "ran out of memory!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
        exit(1);
    }
    glNamedBufferSubData(vertexbuffer, num_verts*3*sizeof(GLfloat), newvecs->verts.size()*sizeof(GLfloat), &newvecs->verts.front());
    glNamedBufferSubData(uvbuffer, num_verts*2*sizeof(GLfloat), newvecs->uvs.size()*sizeof(GLfloat), &newvecs->uvs.front());
    glNamedBufferSubData(lightbuffer, num_verts*sizeof(GLfloat), newvecs->light.size()*sizeof(GLfloat), &newvecs->light.front());
    glNamedBufferSubData(matbuffer, num_verts*sizeof(GLint), newvecs->mats.size()*sizeof(GLfloat), &newvecs->mats.front());
    
    num_verts += newvecs->num_verts;
    return pair<int,int>(old_num_verts/6, newvecs->num_verts/6);
}

void GLVecs::del(pair<int,int> index) {
    //cout << "del: " << index.first << ' ' << index.second << endl;
    if ( index.second != 0) {
        empty.push_back(index);
    }
}

void GLVecs::clean() {
    for (pair<int,int> index : empty) {
        int start = index.first*6;
        int size = index.second*6;
        GLfloat zerosf[size*3] = {0.0f};
        GLint zerosi[size] = {0};
        glNamedBufferSubData(vertexbuffer, start*3*sizeof(GLfloat), size*3*sizeof(GLfloat), zerosf);
        glNamedBufferSubData(uvbuffer, start*2*sizeof(GLfloat), size*2*sizeof(GLfloat), zerosf);
        glNamedBufferSubData(lightbuffer, start*sizeof(GLfloat), size*sizeof(GLfloat), zerosf);
        glNamedBufferSubData(matbuffer, start*sizeof(GLint), size*sizeof(GLint), zerosi);
    }
}

void GLVecs::edit( pair<int,int> index, RenderVecs* newvecs) {
    cout << "editing!" << endl;
    int location = index.first*6;
    glNamedBufferSubData(vertexbuffer, location*3*sizeof(GLfloat), newvecs->verts.size()*sizeof(GLfloat), &newvecs->verts.front());
    glNamedBufferSubData(uvbuffer, location*2*sizeof(GLfloat), newvecs->uvs.size()*sizeof(GLfloat), &newvecs->uvs.front());
    glNamedBufferSubData(lightbuffer, location*sizeof(GLfloat), newvecs->light.size()*sizeof(GLfloat), &newvecs->light.front());
    glNamedBufferSubData(matbuffer, location*sizeof(GLint), newvecs->mats.size()*sizeof(GLfloat), &newvecs->mats.front());
}

void GLVecs::status(std::stringstream & message) {
    message << "-----memory vectors-----" << endl;
    message << "num verts: " << num_verts/6 << endl;
    message << "allocated: " << size_alloc/6 << endl;
    int sum;
    for (pair<int,int> f : empty) {
        sum += f.second;
    }
    message << "empty: " << sum << endl;
}

#endif
