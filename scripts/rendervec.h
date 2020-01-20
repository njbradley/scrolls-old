#ifndef RENDERVEC
#define RENDERVEC
#include <iostream>
#include <GL/glew.h>
using std::cout;
using std::endl;
#include <chrono>

class RenderVecs {
    public:
        vector<GLfloat> verts;
    	vector<GLfloat> uvs;
    	vector<GLfloat> light;
    	vector<GLint> mats;
        int num_verts = 0;
        
        void add_face(GLfloat* newverts, GLfloat* newuvs, GLfloat newlight, GLint mat) {
            verts.insert(verts.end(), newverts, newverts+6*3);
            uvs.insert(uvs.end(), newuvs, newuvs+6*2);
            for (int i = 0; i < 6; i ++) {
                light.push_back(newlight);
                mats.push_back(mat);
            }
            num_verts += 6;
        }
        
};


class GLVecs {
    public:
        GLuint vertexbuffer = 4545;
        GLuint uvbuffer;
        GLuint lightbuffer;
        GLuint matbuffer;
        vector<float> empty;
        int size_alloc;
        int num_verts = 0;
        
        void set_buffers(GLuint verts, GLuint uvs, GLuint light, GLuint mats, int start_size) {
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
            
            
            
            cout << "err: " << std::hex << glGetError() << std::dec << endl;
        }
        
        float add(RenderVecs* newvecs) {
            int old_num_verts = num_verts;
            int location = num_verts;
            
            for (int i = 0; i < empty.size(); i ++) {
                int num_faces = (int)(empty[i]*10)%10;
                if (num_faces == newvecs->num_verts/6) {
                    location = (int)empty[i]*6;
                    float index = empty[i];
                    //cout << "used: " << empty[i] << " for " << newvecs->num_verts << endl;
                    empty.erase(empty.begin()+i);
                    
                    //double before = clock();
                    glNamedBufferSubData(vertexbuffer, location*3*sizeof(GLfloat), newvecs->verts.size()*sizeof(GLfloat), &newvecs->verts.front());
                    glNamedBufferSubData(uvbuffer, location*2*sizeof(GLfloat), newvecs->uvs.size()*sizeof(GLfloat), &newvecs->uvs.front());
                    glNamedBufferSubData(lightbuffer, location*sizeof(GLfloat), newvecs->light.size()*sizeof(GLfloat), &newvecs->light.front());
                    glNamedBufferSubData(matbuffer, location*sizeof(GLint), newvecs->mats.size()*sizeof(GLfloat), &newvecs->mats.front());
                    //cout << clock()-before << endl;
                    /*
                    for (int i = 0; i < newvecs->verts.size(); i ++) {
                        verts[location*3+i] = newvecs->verts[i];
                    }
                    for (int i = 0; i < newvecs->uvs.size(); i ++) {
                        uvs[location*2+i] = newvecs->uvs[i];
                    }
                    for (int i = 0; i < newvecs->light.size(); i ++) {
                        light[location+i] = newvecs->light[i];
                    }
                    for (int i = 0; i < newvecs->mats.size(); i ++) {
                        mats[location+i] = newvecs->mats[i];
                    }*/
                    //num_verts += newvecs->num_verts;
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
            /*    
            verts.insert(verts.end(), newvecs->verts.begin(), newvecs->verts.end());
            uvs.insert(uvs.end(), newvecs->uvs.begin(), newvecs->uvs.end());
            light.insert(light.end(), newvecs->light.begin(), newvecs->light.end());
            mats.insert(mats.end(), newvecs->mats.begin(), newvecs->mats.end());
            */
            num_verts += newvecs->num_verts;
            float out = old_num_verts/6 + newvecs->num_verts/6*0.1f;
            //cout << "add:" << out << endl;
            //cout << out << ' ' << old_num_verts <<  ' '<< newvecs->num_verts << endl;
            return out;
        }
        
        void del(float index) {
            //cout << "del: " << index << endl;
            int faces = (int)(index*10)%10;
            if ( faces != 0) {
                empty.push_back(index);
                int start = (int)index*6;
                int size = (int)(index*10)%10*6;
                GLfloat zerosf[size*3] = {0.0f};
                GLint zerosi[size] = {0};
                glNamedBufferSubData(vertexbuffer, start*3*sizeof(GLfloat), size*3*sizeof(GLfloat), zerosf);
                glNamedBufferSubData(uvbuffer, start*2*sizeof(GLfloat), size*2*sizeof(GLfloat), zerosf);
                glNamedBufferSubData(lightbuffer, start*sizeof(GLfloat), size*sizeof(GLfloat), zerosf);
                glNamedBufferSubData(matbuffer, start*sizeof(GLint), size*sizeof(GLint), zerosi);
                /*
                for (; start < end; start += 6) {
                    for (int i = 0; i < 6*3; i ++) {
                        verts[start*3+i] = 0;//verts[extras*3+i];
                    }
                    for (int i = 0; i < 6*2; i ++) {
                        uvs[start*2+i] = 0;//uvs[extras*2+i];
                    }
                    for (int i = 0; i < 6; i ++) {
                        light[start+i] = 0;//light[extras+i];
                    }
                    for (int i = 0; i < 6; i ++) {
                        mats[start+i] = 0;//mats[extras+i];
                    }
                }*/
                //num_verts -= (int)(index*10)%10*6;
            }
        }
        
        void clean() {
            return;
        }
        
        void status(std::stringstream & message) {
            message << "-----memory vectors-----" << endl;
            message << "num verts: " << num_verts/6 << endl;
            message << "allocated: " << size_alloc/6 << endl;
            message << "empty: ";
            int sum;
            for (float f : empty) {
                //message << f << ' ';
                sum += (int)(f*10)%10;
            }
            message << sum << endl << "size: " << num_verts/6 << endl;
        }
                    
};
#endif