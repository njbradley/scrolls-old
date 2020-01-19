#include <iostream>
using std::cout;
using std::endl;


class RenderVecs {
    public:
        vector<GLfloat> verts;
    	vector<GLfloat> uvs;
    	vector<GLfloat> light;
    	vector<GLint> mats;
        vector<float> empty;
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


class GlVecs {
    public:
        
        float add(RenderVecs* newvecs) {
            int old_num_verts = light.size();
            int location = light.size();
            
            for (int i = 0; i < empty.size(); i ++) {
                int num_faces = (int)(empty[i]*10)%10;
                if (num_faces == newvecs->num_verts/6) {
                    location = (int)empty[i]*6;
                    float index = empty[i];
                    //cout << "used: " << empty[i] << " for " << newvecs->num_verts << endl;
                    empty.erase(empty.begin()+i);
                    
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
                    }
                    //num_verts += newvecs->num_verts;
                    return index;
                }
            }
            verts.insert(verts.end(), newvecs->verts.begin(), newvecs->verts.end());
            uvs.insert(uvs.end(), newvecs->uvs.begin(), newvecs->uvs.end());
            light.insert(light.end(), newvecs->light.begin(), newvecs->light.end());
            mats.insert(mats.end(), newvecs->mats.begin(), newvecs->mats.end());
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
                int end = start + (int)(index*10)%10*6;
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
                }
                //num_verts -= (int)(index*10)%10*6;
            }
        }
        
        void clean() {
            return;
            int extras = num_verts;
            for (float f : empty) {
                cout << f << ' ' << extras/6 << ' ' << light.size()/6 << endl;
                int start = (int)f*6;
                if (start > num_verts) {
                    extras += (int)(f*10)%10*6;
                    continue;
                }
                int end = start + (int)(f*10)%10*6;
                cout << start/6 << ' ' << end/6 << endl;
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
                    extras += 6;
                }
            }
        }
        
        void status(std::stringstream & message) {
            message << "memory vectors:" << endl;
            message << "num verts: " << num_verts/6 << endl;
            message << "empty: ";
            int sum;
            for (float f : empty) {
                //message << f << ' ';
                sum += (int)(f*10)%10;
            }
            message << sum << endl << "size: " << light.size()/6 << endl;
        }
                    
};