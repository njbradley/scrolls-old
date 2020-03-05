#ifndef BLOCKS
#define BLOCKS

#include <iostream>
#include <functional>
#include <cmath>
#include <fstream>
#include <sstream>
#include <thread>
#include "cross-platform.h"
#include "rendervec.h"
#include "blockdata-predef.h"
#include "blocks-predef.h"
#include "world-predef.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <map>
using namespace glm;
using std::thread;
#include "generative.h"

#define csize 2


/*
 *this is the file with classes for storing blocks. the main idea is blocks are stored in power of two containers. Block is the base virtual class,
 *and Pixel and Chunk derive from Block.
 *
 */

//////////////////////////// class Block /////////////////////////////////////

Block::Block(int x, int y, int z, int newscale, Chunk* newparent) :
    px(x), py(y), pz(z), scale(newscale), parent(newparent) {}

void Block::world_to_local(int x, int y, int z, int* lx, int* ly, int* lz) {
    *lx = x-(x/scale*scale);
    *ly = y-(y/scale*scale);
    *lz = z-(z/scale*scale);
    //cout << scale << " -----scale ------" << endl;
}

void Block::global_position(int* gx, int* gy, int* gz) {
    //cout << px << ' ' << py << ' ' << pz << ' ' << scale << endl;
    if (parent == nullptr) {
        *gx = px*scale;
        *gy = py*scale;
        *gz = pz*scale;
    } else {
        parent->global_position(gx, gy, gz);
        *gx += px*scale;
        *gy += py*scale;
        *gz += pz*scale;
    }
    //cout << *gx << ' ' << *gy << ' ' << *gz << endl;
}

Block* Block::get_world() {
    if (parent == nullptr) {
        return this;
    } else {
        return parent->get_world();
    }
}

Block* Block::raycast(double* x, double* y, double* z, double dx, double dy, double dz, double time) {
    //cout << "block raycast(" << *x << ' ' << *y << ' ' << *z << ' ' << dx << ' ' << dy << ' ' << dz << endl;
    if (!continues() and get() != 0) {
        //cout << "returning" << continues() << get() << endl;
        return this;
    }
    
    
    double lx, ly, lz;
    //world_to_local(*x, *y, *z, &lx, &ly, &lz);
    int gx, gy, gz;
    global_position(&gx, &gy, &gz);
    
    lx = *x - gx;
    ly = *y - gy;
    lz = *z - gz;
    
    //cout << "this " << px <<  ' ' << py << ' ' << pz << ' ' << scale << endl;
    //cout << "local " << lx << ' ' << ly << ' ' << lz << endl;
    //cout << "global " << gx << ' ' << gy << ' ' << gz << endl;
    
    double tx = (dx < 0) ? lx/(-dx) : (scale-lx)/dx;
    double ty = (dy < 0) ? ly/(-dy) : (scale-ly)/dy;
    double tz = (dz < 0) ? lz/(-dz) : (scale-lz)/dz;
    
    //cout << "ts " << tx << ' ' << ty << ' ' << tz << endl;
    double extra = 0.0001;
    
    if (tx < ty and tx < tz) { // hits x wall first
        *x += dx*(tx+extra);
        *y += dy*(tx+extra);
        *z += dz*(tx+extra);
        time -= tx;
    } else if (ty < tx and ty < tz) { // hits y wall first
        *x += dx*(ty+extra);
        *y += dy*(ty+extra);
        *z += dz*(ty+extra);
        time -= ty;
    } else if (tz < tx and tz < ty) { // hits z wall first
        *x += dx*(tz+extra);
        *y += dy*(tz+extra);
        *z += dz*(tz+extra);
        time -= tz;
    }
    time -= 0.01;
    //cout << "get(" << (int)*x - (*x<0) << ' ' << (int)*y - (*y<0) << ' ' << (int)*z - (*z<0) << ' ' << (*x < 0) << endl;
    Block* b = world->get_global((int)*x - (*x<0), (int)*y - (*y<0), (int)*z - (*z<0), 1);
    if (b == nullptr or time < 0) {
        return nullptr;
    }
    return b->raycast(x, y, z, dx, dy, dz, time);
}





///////////////////////////////// CLASS PIXEL /////////////////////////////////////

Pixel::Pixel(int x, int y, int z, char new_val, int nscale, Chunk* nparent):
  Block(x, y, z, nscale, nparent), render_index(-1,0), value(new_val) {
    if (value != 0) {
      lightlevel = blocks->blocks[value]->lightlevel;
      if (blocks->blocks[value]->extras != nullptr) {
        //extras = new BlockExtras(this);
      }
    } else {
      lightlevel = 0;
    }
    lightlevel = 1;
}

bool Pixel::continues() {
    return false;
}

char Pixel::get() {
    //cout << "pixel get px" << px << " py" << py << " pz" << pz << " s" << scale << " value" << value  << endl;
    return value;
}

void Pixel::all(function<void(Pixel*)> func) {
    func(this);
}

void Pixel::all_side(function<void(Pixel*)> func, int dx, int dy, int dz) {
    func(this);
}

Pixel* Pixel::get_pix() {
    return this;
}

float Pixel::get_lightlevel(int dx, int dy, int dz) {
    return lightlevel;
}

Block * Pixel::get(int x, int y, int z) {
    cout << "error: get(int,int,int) called on pixel" << endl;
    return nullptr;
}

bool Pixel::is_air(int dx, int dy, int dz) {
    //cout << "pix is air reaturning:" << (value == 0) << endl;
    return value == 0;
}

Block* Pixel::get_global(int x, int y, int z, int scale) {
    //cout << "get global at pixel retunring self\n\n";
    return this;
}

void Pixel::set(char val) {
    
    //cout << "render_indes: " << render_index << " val:" << (int)value <<  ' '<< (int)val << endl;
    //cout << "pixel(" << (int)value <<  " scale " << scale << ") set " << (int)val << endl;
    value = val;
    render_flag = true;
    int gx, gy, gz;
    global_position(&gx, &gy, &gz);
    //cout << gx << ' ' << gy << ' ' << gz << "-----------------------------" << render_index.first << ' ' << render_index.second << endl;
    //cout << "global position " << gx << ' ' << gy << ' ' << gz << endl;
    render_update();
    world->get_global(gx+scale, gy, gz, scale)->render_update();
    world->get_global(gx-scale, gy, gz, scale)->render_update();
    world->get_global(gx, gy+scale, gz, scale)->render_update();
    world->get_global(gx, gy-scale, gz, scale)->render_update();
    world->get_global(gx, gy, gz+scale, scale)->render_update();
    world->get_global(gx, gy, gz-scale, scale)->render_update();
    
}

void Pixel::render_update() {
    //int gx, gy, gz;
    //global_position(&gx, &gy, &gz);
    //cout << gx << ' ' << gy << ' ' << gz << endl;
    //cout << render_index << endl;
    //cout << "render_update() " << render_index.first << ' ' << render_index.second << endl;
    //lightlevel = -1;
    if (render_index.first > -1) {
        world->glvecs.del(render_index);
        render_index = pair<int,int>(-1,0);
        //cout << render_index.first << ' ' << render_index.second << " x" << endl;
        //render_index.first = -(render_index.first+2);
    }
}

void Pixel::del() {
    world->glvecs.del(render_index);
    if (extras != nullptr) {
      delete extras;
      cout << "dleleted extras" << endl;
    }
}

void Pixel::lighting_update() {
    
}


void Pixel::calculate_light_level() {
  int gx, gy, gz;
  global_position(&gx, &gy, &gz);
  //cout << gx << ' ' << gy << ' ' << gz << ' ' << scale << endl;
  for (int axis = 0; axis < 3; axis ++) {
    for (int dir = -1; dir < 2; dir += 2) {
      int offset[3] = {0,0,0};
      offset[axis] += dir;
      Block* other = world->get_global(offset[0]*scale+gx, offset[1]*scale+gy, offset[2]*scale+gz, scale);
      if (other == nullptr) {
        continue;
      }
      other->all_side([&] (Pixel* pix) {
        //cout << pix->get_lightlevel(-offset[0], -offset[1], -offset[2]) << ',' << lightlevel << endl;
        if (pix->value == 0 and pix->get_lightlevel(-offset[0], -offset[1], -offset[2]) < lightlevel) {
          pix->lightlevel = lightlevel*0.8f;
          //cout << "setting light level" << endl;
          pix->calculate_light_level();
        }
      }, -offset[0], -offset[1], -offset[2]);
      //cout << offset[0] << ' ' << offset[1] << ' ' << offset[2] << endl;
    }
  }
}

void Pixel::render(GLVecs* allvecs, int gx, int gy, int gz) {
    if (value == 0 or render_index.first != -1) {
        return;
    }
    gx += px*scale;
    gy += py*scale;
    gz += pz*scale;
    GLfloat x = gx;
    GLfloat y = gy;
    GLfloat z = gz;
    //cout << x << ' ' << y << ' ' << z << "rendering call" << endl;
    GLfloat uv_start = 0.0f;// + (1/32.0f*(int)value);
    GLfloat uv_end = uv_start + 1.0f;///32.0f;
    char mat = blocks->blocks[value]->texture;
    int minscale = blocks->blocks[value]->minscale;
    GLfloat new_uvs[] = {
        0.0f, 1.0f * scale/minscale,
        1.0f * scale/minscale, 1.0f * scale/minscale,
        1.0f * scale/minscale, 0.0f,
        1.0f * scale/minscale, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f * scale/minscale
    };
    RenderVecs vecs;
    //cout << 85 << endl;
    Block* block;
    //cout << 87 << endl;
    
    block = world->get_global(gx, gy, gz-scale, scale);
    //exit(2);
    //if (block != nullptr) {
    //    cout << scale << ' ' << block->scale << " sd-------------------------------------\n";
    //    int ox, oy, oz;
    //    block->global_position(&ox, &oy, &oz);
    //   cout << ox << ' ' << oy << ' ' << oz << ' ' << block->px << ' ' << block->py << ' ' << block->pz << endl;
    //    //exit(3);
    //
    if (block == nullptr or block->is_air(0, 0, 1)) {
        GLfloat face[] = {
            x + scale, y, z,
            x, y, z,
            x, y + scale, z,
            x, y + scale, z,
            x + scale, y + scale, z,
            x + scale, y, z
        };
        vecs.add_face(face, new_uvs, 0.7f * ( (block!=nullptr) ? block->get_lightlevel(0,0,1) : 1 ), mat);//0.4f
    }
    
    block = world->get_global(gx, gy-scale, gz, scale);
    if (block == nullptr or block->is_air(0, 1, 0)) {
        GLfloat face[] = {
            x, y, z,
            x + scale, y, z,
            x + scale, y, z + scale,
            x + scale, y, z + scale,
            x, y, z + scale,
            x, y, z
        };
        
        vecs.add_face(face, new_uvs, 0.5f * ( (block!=nullptr) ? block->get_lightlevel(0,1,0) : 1 ), mat);
    }
    
    block = world->get_global(gx-scale, gy, gz, scale);
    if (block == nullptr or block->is_air(1, 0, 0)) {
        GLfloat face[] = {
            x, y, z,
            x, y, z + scale,
            x, y + scale, z + scale,
            x, y + scale, z + scale,
            x, y + scale, z,
            x, y, z
        };
        vecs.add_face(face, new_uvs, 0.7f * ( (block!=nullptr) ? block->get_lightlevel(1,0,0) : 1 ), mat);
    }
    
    block = world->get_global(gx, gy, gz+scale, scale);
    if (block == nullptr or block->is_air(0, 0, -1)) {
        GLfloat face[] = {
            x, y, z + scale,
            x + scale, y, z + scale,
            x + scale, y + scale, z + scale,
            x + scale, y + scale, z + scale,
            x, y + scale, z + scale,
            x, y, z + scale
        };
        vecs.add_face(face, new_uvs, 0.7f * ( (block!=nullptr) ? block->get_lightlevel(0,0,-1) : 1 ), mat);
    }
    
    block = world->get_global(gx, gy+scale, gz, scale);
    if (block == nullptr or block->is_air(0, -1, 0)) {
        GLfloat face[] = {
            x + scale, y + scale, z + scale,
            x + scale, y + scale, z,
            x, y + scale, z,
            x, y + scale, z,
            x, y + scale, z + scale,
            x + scale, y + scale, z + scale
        };
        vecs.add_face(face, new_uvs, 0.9f * ( (block!=nullptr) ? block->get_lightlevel(0,-1,0) : 1 ), mat);//top
    }
    
    block = world->get_global(gx+scale, gy, gz, scale);
    if (block == nullptr or block->is_air(-1, 0, 0)) {
        GLfloat face[] = {
            x + scale, y, z + scale,
            x + scale, y, z,
            x + scale, y + scale, z,
            x + scale, y + scale, z,
            x + scale, y + scale, z + scale,
            x + scale, y, z + scale
        };
        vecs.add_face(face, new_uvs, 0.7f * ( (block!=nullptr) ? block->get_lightlevel(-1,0,0) : 1 ), mat); //0.4f
    }
    render_index = allvecs->add(&vecs);
    
    //cout << render_index << endl;
    //cout << "done rendering" << endl;
    //cout << *num_verts << endl;
    //exit(1);
}

Chunk* Pixel::subdivide(function<void(int,int,int,Pixel*)> func) {
    //cout << "subdivide " << endl;
    //render_update();
    if (render_index.first != -1) {
        world->glvecs.del(render_index);
    }
    if (scale == 1) {
        cout << "error: block alreasy at scale 1" << endl;
        return nullptr;
    }
    Chunk * newchunk = new Chunk(px, py, pz, scale, parent);
    for (int x = 0; x < csize; x ++) {
        for (int y = 0; y < csize; y ++) {
            for (int z = 0; z < csize; z ++) {
                int gx, gy, gz;
                Pixel * pix = new Pixel(x, y, z, value, scale/csize, newchunk);
                pix->global_position(&gx, &gy, &gz);
                newchunk->blocks[x][y][z] = pix;
                func(gx,gy,gz,pix);
            }
        }
    }
    //cout << '(' << parent->get(px,py,pz)->get() << ' ' << value << ')' << endl;
    //cout << "ysay " << endl;
    if (parent != nullptr) {
        //cout << parent << ' ' << parent->blocks[px][py][pz] << endl;
        parent->blocks[px][py][pz] = newchunk;
    }
    //cout << this << " this" << endl;
    //delete this;
    //cout << 678 << endl;
    return newchunk;
}

void Pixel::save_to_file(ofstream* of) {
    *of << value;
}


/////////////////////////////////////// CLASS CHUNK ///////////////////////////////////////

Chunk::Chunk(int x, int y, int z, int nscale, Chunk* nparent):
    Block(x, y, z, nscale, nparent) {}

bool Chunk::continues() {
    return true;
}

Block * Chunk::get(int x, int y, int z) {
    //cout << "chunk get px" << px << " py" << py << " pz" << pz << " s" << scale << " x" << x << " y" << y << " z" << z << endl;
    return blocks[x][y][z];
}

Pixel* Chunk::get_pix() {
    return nullptr;
}

char Chunk::get() {
    cout << "error: get() called on chunk object" << endl;
}
void Chunk::set(char c) {
    cout << "error: set called on chunk object" << endl;
}
Chunk* Chunk::subdivide(function<void(int,int,int,Pixel*)> func) {
    cout << "error: subdividing already subdivided block" << endl;
    return nullptr;
}
void Chunk::calculate_light_level() {
    for (int x = 0; x < csize; x ++) {
        for (int y = csize-1; y >= 0; y --) {
            for (int z = 0; z < csize; z ++) {
                blocks[x][y][z]->calculate_light_level();
            }
        }
    }
}
void Chunk::all(function<void(Pixel*)> func) {
    for (int x = 0; x < csize; x ++) {
        for (int y = csize-1; y >= 0; y --) {
            for (int z = 0; z < csize; z ++) {
                blocks[x][y][z]->all(func);
            }
        }
    }
}

void Chunk::all_side(function<void(Pixel*)> func, int dx, int dy, int dz) {
    int x_start = 0;
    int x_end = csize;
    int y_start = 0;
    int y_end = csize;
    int z_start = 0;
    int z_end = csize;
    if (dx != 0) {
        x_start = (int)( dx/2.0f + 0.5f );
        x_end = 1 + x_start;
    }
    if (dy != 0) {
        y_start = (int)( dy/2.0f + 0.5f );
        y_end = 1 + y_start;
    }
    if (dz != 0) {
        z_start = (int)( dz/2.0f + 0.5f );
        z_end = 1 + z_start;
    }
    for (int x = x_start; x < x_end; x ++) {
        for (int y = y_start; y < y_end; y ++) {
            for (int z = z_start; z < z_end; z ++) {
                blocks[x][y][z]->all_side(func, dx, dy, dz);
            }
        }
    }
}

float Chunk::get_lightlevel(int dx, int dy, int dz) {
    int x_start = 0;
    int x_end = csize;
    int y_start = 0;
    int y_end = csize;
    int z_start = 0;
    int z_end = csize;
    if (dx != 0) {
        x_start = (int)( dx/2.0f + 0.5f );
        x_end = 1 + x_start;
    }
    if (dy != 0) {
        y_start = (int)( dy/2.0f + 0.5f );
        y_end = 1 + y_start;
    }
    if (dz != 0) {
        z_start = (int)( dz/2.0f + 0.5f );
        z_end = 1 + z_start;
    }
    
    float lightlevel = 0;
    int num = 0;
    
    //cout << "for loop:" << endl;
    for (int x = x_start; x < x_end; x ++) {
        for (int y = y_start; y < y_end; y ++) {
            for (int z = z_start; z < z_end; z ++) {
                float level = blocks[x][y][z]->get_lightlevel(dx, dy, dz);
                if (level == -1) {
                    cout << "blocks.h error no: 4859347592579493845748395766" << endl;
                    exit(1);
                    return -1;
                }
                if (blocks[x][y][z]->is_air(dx,dy,dz)) {
                    if (level != 1) {
                    //cout << level << ' ' << x << ' ' << y << ' ' << z << endl;
                }
                    lightlevel += level;
                    num ++;
                }
            }
        }
    }
    if (num == 0) {
        return 0;
    } else {
        return  lightlevel/num;
    }
}

void Chunk::render(GLVecs* vecs, int gx, int gy, int gz) {
    for (int x = 0; x < csize; x ++) {
        for (int y = 0; y < csize; y ++) {
            for (int z = 0; z < csize; z ++) {
                blocks[x][y][z]->render(vecs, gx + px*scale, gy + py*scale, gz + pz*scale);
            }
        }
    }
}

void Chunk::del() {
    for (int x = 0; x < csize; x ++) {
        for (int y = 0; y < csize; y ++) {
            for (int z = 0; z < csize; z ++) {
                blocks[x][y][z]->del();
                delete blocks[x][y][z];
            }
        }
    }
}

void Chunk::render_update() {
    for (int x = 0; x < csize; x ++) {
        for (int y = 0; y < csize; y ++) {
            for (int z = 0; z < csize; z ++) {
                blocks[x][y][z]->render_update();
            }
        }
    }
}

Block* Chunk::get_global(int x, int y, int z, int nscale) {
    //cout << "blocks get_global " << x << ' ' << y << ' ' << z << " scale:" << nscale << endl;
    if (nscale == scale) {
        //cout << " at scale return this\n\n";
        return this;
    } else {
        if (x < 0 or x >= scale or y < 0 or y >= scale or z < 0 or z >= scale) {
            //cout << "ret null ---s -- - - -- -" << endl;
            //return nullptr;
        }
        int lx, ly, lz;
        //cout << 206 << endl;
        world_to_local(x, y, z, &lx, &ly, &lz);
        //cout << lx << ' ' << ly << ' ' << lz << endl;
        if (lx < 0 or lx >= csize*scale or ly < 0 or ly >= csize*scale or lz < 0 or lz >= csize*scale) {
            //cout << "returning null -------------------------------\n\n";
            return nullptr;
        }
        int nlx = lx/(scale/csize);
        int nly = ly/(scale/csize);
        int nlz = lz/(scale/csize);
        //cout << nlx << ' ' << nly << ' ' << nlz << endl;
        //cout << "sdfkhaskdfjhaksldfhalkjsdfhalksfh 229 ksjdfl;akjfd;alksdjf;alksdfja;sd" << endl;
        return get(nlx, nly, nlz)->get_global(lx, ly, lz, nscale);
    }
}
            

bool Chunk::is_air(int dx, int dy, int dz) {
    //cout << "is_air(dx" << dx << " dy" << dy << " dz" << dz << ")\n";
    int gx, gy, gz;
    global_position(&gx, &gy, &gz);
    //cout << gx << ' ' << gy << ' ' << gz << endl;
    bool air = false;
    int x_start = 0;
    int x_end = csize;
    int y_start = 0;
    int y_end = csize;
    int z_start = 0;
    int z_end = csize;
    if (dx != 0) {
        x_start = (int)( dx/2.0f + 0.5f );
        x_end = 1 + x_start;
    }
    if (dy != 0) {
        y_start = (int)( dy/2.0f + 0.5f );
        y_end = 1 + y_start;
    }
    if (dz != 0) {
        z_start = (int)( dz/2.0f + 0.5f );
        z_end = 1 + z_start;
    }
    //cout << "for loop:" << endl;
    for (int x = x_start; x < x_end; x ++) {
        for (int y = y_start; y < y_end; y ++) {
            for (int z = z_start; z < z_end; z ++) {
                //cout << x << ' ' << y << ' ' << z << ' ' << get(x, y, z)->is_air(dx, dy, dz) << endl;
                air = air or get(x, y, z)->is_air(dx, dy, dz);
            }
        }
    }
    //cout << "returning:" << air << endl << endl;
    return air;
}

void Chunk::save_to_file(ofstream* of) {
    *of << "{";
    for (int x = 0; x < csize; x ++) {
        for (int y = 0; y < csize; y ++) {
            for (int z = 0; z < csize; z ++) {
                blocks[x][y][z]->save_to_file(of);
            }
        }
    }
    *of << "}";
}

#endif
