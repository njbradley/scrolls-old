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
#include "tiles-predef.h"
#include "blockphysics-predef.h"
#include "blocks-predef.h"
#include "world-predef.h"

#include <GL/glew.h>
#include <map>
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
    px(x), py(y), pz(z), scale(newscale), parent(newparent) {
    set_render_flag();
}

void Block::set_render_flag() {
  render_flag = true;
  if (parent != nullptr and !parent->render_flag) {
    parent->set_render_flag();
  }
}

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

vec3 Block::get_position() {
  int x,y,z;
  global_position(&x,&y,&z);
  return vec3(x,y,z);
}

Block* Block::raycast(Collider* this_world, double* x, double* y, double* z, double dx, double dy, double dz, double time) {
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
    Block* b = this_world->get_global((int)*x - (*x<0), (int)*y - (*y<0), (int)*z - (*z<0), 1);
    if (b == nullptr or time < 0) {
        return nullptr;
    }
    return b->raycast(this_world, x, y, z, dx, dy, dz, time);
}

Block* Block::from_file(istream& ifile, int px, int py, int pz, int scale, Chunk* parent, Tile* tile) {
    char c;
    ifile.read(&c,1);
    if (c == '{') {
        Chunk* chunk = new Chunk(px, py, pz, scale, parent);
        for (int x = 0; x < csize; x ++) {
            for (int y = 0; y < csize; y ++) {
                for (int z = 0; z < csize; z ++) {
                    chunk->blocks[x][y][z] = Block::from_file(ifile, x, y, z, scale/csize, chunk, tile);
                }
            }
        }
        ifile.read(&c,1);
        if (c != '}') {
            cout << "error, mismatched {} in save file" << endl;
        }
        return (Block*) chunk;
    } else {
        if (c == '~') {
          c = ifile.get();
          BlockExtra* extra = new BlockExtra(ifile);
          Pixel* p = new Pixel(px, py, pz, c, scale, parent, tile, extra);
          return (Block*) p;
        } else {
          int direction = -1;
          if (c >= 100) {
            direction = c - 100;
            c = ifile.get();
          }
          Pixel* p = new Pixel(px, py, pz, c, scale, parent, tile);
          if (direction != -1) {
            p->direction = direction;
          }
          return (Block*) p;
        }
    }
}



///////////////////////////////// CLASS PIXEL /////////////////////////////////////

Pixel::Pixel(int x, int y, int z, char new_val, int nscale, Chunk* nparent, Tile* ntile):
  Block(x, y, z, nscale, nparent), render_index(-1,0), value(new_val), extras(nullptr), physicsgroup(nullptr), tile(ntile), direction(0) {
    reset_lightlevel();
    generate_extras();
    if (value != 0) {
      direction = blocks->blocks[value]->default_direction;
    }
}

Pixel::Pixel(int x, int y, int z, char new_val, int nscale, Chunk* nparent, Tile* ntile, BlockExtra* new_extra):
  Block(x, y, z, nscale, nparent), render_index(-1,0), value(new_val), extras(new_extra), physicsgroup(nullptr), tile(ntile), direction(0) {
    reset_lightlevel();
    if (value != 0) {
      direction = blocks->blocks[value]->default_direction;
    }
}

void Pixel::generate_extras() {
  if (extras != nullptr) {
    delete extras;
  }
  if (value != 0) {
    if (blocks->blocks[value]->extras != nullptr) {
      extras = new BlockExtra(this);
    }
  }
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
  return 1;
    return lightlevel;
}

bool Pixel::is_air(int dx, int dy, int dz) {
    //cout << "pix is air reaturning:" << (value == 0) << endl;
    return value == 0;
}

Block* Pixel::get_global(int x, int y, int z, int scale) {
    //cout << "get global at pixel retunring self\n\n";
    return this;
}

void Pixel::set(char val, int newdirection, BlockExtra* newextras, bool update) {
    int gx, gy, gz;
    global_position(&gx, &gy, &gz);
    //cout << "render_indes: " << render_index << " val:" << (int)value <<  ' '<< (int)val << endl;
    //cout << "pixel(" << (int)value <<  " scale " << scale << ") set " << (int)val << endl;
    if (val == 0) {
      newdirection = 0;
    } else {
      if (newdirection == -1 or !blocks->blocks[val]->rotation_enabled) {
        newdirection = blocks->blocks[val]->default_direction;
      }
    }
    
    value = val;
    if (newextras == nullptr) {
      generate_extras();
    } else {
      delete extras;
      extras = newextras;
    }
    direction = newdirection;
    
    if (physicsgroup != nullptr) {
      physicsgroup->update_flag = true;//block_poses.erase(ivec3(gx,gy,gz));
      //physicsgroup = nullptr;
    }
    physicsgroup = nullptr;
    if (update) {
      //render_flag = true;
      //cout << gx << ' ' << gy << ' ' << gz << "-----------------------------" << render_index.first << ' ' << render_index.second << endl;
      //cout << "global position " << gx << ' ' << gy << ' ' << gz << endl;
      reset_lightlevel();
      render_update();
      Block* block;
      const ivec3 dir_array[6] =    {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
      for (ivec3 dir : dir_array) {
        dir *= scale;
        block = world->get_global(gx+dir.x, gy+dir.y, gz+dir.z, scale);
        if (block != nullptr) {
          block->render_update();
        }
      }
    }
}

void Pixel::tick() {
  if (tile == nullptr) {
    cout << "err" << endl;
  }
  int gx, gy, gz;
  global_position(&gx, &gy, &gz);
  //cout << "START PIX tick --------------------------";
  //print (ivec3(gx, gy, gz));
  if (physicsgroup == nullptr) {
    BlockGroup* group = BlockGroup::make_group(value, tile->world, ivec3(gx, gy, gz));
    //cout << group->block_poses.size() << " size!" << endl;
    if (group->block_poses.size() > 0) {
      if (!group->consts[4]) {
        for (int i = 0; i < 6; i ++) {
          cout << group->consts[i] << ' ';
        } cout << endl;
        cout << "copying " << group->block_poses.size() << endl;
        group->copy_to_block();
        cout << "erasing" << endl;
        group->erase_from_world();
        cout << "done" << endl;
        tile->entities.push_back(new FallingBlockEntity(tile->world, group));
      } else {
        if (group->persistant()) {
          tile->world->physicsgroups.emplace(group);
        }
      }
    } else {
      delete group;
    }
    // else {
      //tile->world->physicsgroups.emplace(group);
    //}
  }
  // if (value == blocks->names["dirt"] and world->get(gx, gy+scale, gz) == 0) {
  //   //set(blocks->names["grass"]);
  // }
  //cout << gx << ' ' << gy << ' ' << gz << "-----------------------------" << render_index.first << ' ' << render_index.second << endl;
  //cout << "global position " << gx << ' ' << gy << ' ' << gz << endl;
  /*
  if (parent != nullptr and extras == nullptr) {
    bool cont = true;
    for (int x = 0; x < csize; x ++) {
      for (int y = 0; y < csize; y ++) {
        for (int z = 0; z < csize; z ++) {
          cont = cont and (!parent->get(x,y,z)->continues()) and parent->get(x,y,z)->get() == value;
        }
      }
    }
    if (cont) {
      if (parent->parent != nullptr) {
        render_update();
        parent->parent->blocks[parent->px][parent->py][parent->pz] = new Pixel(parent->px, parent->py, parent->pz, value, parent->scale, parent->parent, tile);
        parent->del(true);
      }
    }
  }*/
}

void Pixel::render_update() {
    int gx, gy, gz;
    global_position(&gx, &gy, &gz);
    //cout << gx << ' ' << gy << ' ' << gz << endl;
    //cout << render_index << endl;
    //cout << "render_update() " << render_index.first << ' ' << render_index.second << endl;
    //lightlevel = -1;
    //calculate_lightlevel();
        //world->glvecs.del(render_index);
        //render_index = pair<int,int>(-1,0);
        //render_flag = true;
        set_render_flag();
        if (tile != nullptr) {
          tile->world->block_update(gx, gy, gz);
        }
        //cout << render_index.first << ' ' << render_index.second << " x" << endl;
        //render_index.first = -(render_index.first+2);
}

void Pixel::del(bool remove_faces) {
    if (remove_faces) {
      world->glvecs.del(render_index);
    }
    if (extras != nullptr) {
      delete extras;
    }
    if (physicsgroup != nullptr and !physicsgroup->persistant()) {
      delete physicsgroup;
    }
}

void Pixel::lighting_update() {
    
}

void Pixel::reset_lightlevel() {
  if (value != 0) {
    lightlevel = blocks->blocks[value]->lightlevel;
  } else {
    lightlevel = 0;
  }
}

void Pixel::calculate_lightlevel() {
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
        if (pix->value == 0 and pix->get_lightlevel(-offset[0], -offset[1], -offset[2]) < lightlevel*0.9f) {
          pix->lightlevel = lightlevel*0.9f;
          //cout << "setting light level" << endl;
          pix->render_update();
        }
      }, -offset[0], -offset[1], -offset[2]);
      //cout << offset[0] << ' ' << offset[1] << ' ' << offset[2] << endl;
    }
  }
}

void Pixel::rotate(int axis, int dir) {
  const int positive[18] {
    0, 2, 4, 3, 5, 1,
    5, 1, 0, 2, 4, 3,
    1, 3, 2, 4, 0, 5
  };
  const int negative[18] {
    0, 5, 1, 3, 2, 4,
    2, 1, 0, 5, 4, 0,
    4, 0, 2, 1, 3, 5
  };
  if (value != 0 and blocks->blocks[value]->rotation_enabled) {
    if (dir == 1) {
      direction = positive[axis*6 +direction];
    } else if (dir == -1) {
      direction = negative[axis*6 +direction];
    }
  }
  set_render_flag();
}

void Pixel::rotate_uv(GLfloat* uvs, int rot) {
  const GLfloat points[] {uvs[0],uvs[1], uvs[2],uvs[3], uvs[4],uvs[5], uvs[8],uvs[9]};
  const int indexes[] {0,1,2,2,3,0};
  for (int i = 0; i < 6; i ++) {
    int off_index = indexes[i] + rot;
    if (off_index < 0) {
      off_index += 4;
    } if (off_index >= 4) {
      off_index -= 4;
    }
    uvs[i*2] = points[off_index*2];
    uvs[i*2+1] = points[off_index*2+1];
  }
  // for (int i = 0; i < 8; i ++) {
  //   cout << points[i] << ' ';
  // }
  // cout << endl;
  // for (int i = 0; i < 12; i ++) {
  //   cout << uvs[i] << ' ';
  // }
  // cout << endl;
}

void Pixel::rotate_from_origin(int* mats, int* dirs, int rotation) {
  const int mappings[6][6] { // gives the new location of every face for every possible rotation
    {0, 0, 0, 0, 0, 0},
    {1, 3, 2, 4, 0, 5},
    {2, 1, 3, 5, 4, 0},
    {3, 1, 5, 0, 4, 2},
    {4, 0, 2, 1, 3, 5},
    {5, 1, 0, 2, 4, 3}
  };
  const int rot_map[6][6] { // gives the rotation needed for each face
    {0, 0, 0, 0, 0, 0},
    {0, 0, -1, 0, 0, 1},
    {0, 1, 0, 0, -1, 0},
    {0, 2, 0, 0, 2, 0},
    {0, 2, 1, 0, 2, -1},
    {0, -1, 0, 0, 1, 0}
  };
  if (rotation == 0) {
    return;
  }
  int oldmats[6];
  int olddirs[6];
  for (int i = 0; i < 6; i ++) {
    oldmats[i] = mats[i];
    olddirs[i] = dirs[i];
  }
  for (int i = 0; i < 6; i ++) {
    mats[mappings[rotation][i]] = oldmats[i];
    dirs[mappings[rotation][i]] = olddirs[i] + rot_map[rotation][i];
  }
}

void Pixel::rotate_to_origin(int* mats, int* dirs, int rotation) {
  const int mappings[6][6] {
    {0, 0, 0, 0, 0, 0},
    {1, 3, 2, 4, 0, 5},
    {2, 1, 3, 5, 4, 0},
    {3, 1, 5, 0, 4, 2},
    {4, 0, 2, 1, 3, 5},
    {5, 1, 0, 2, 4, 3}
  };
  const int rot_map[6][6] {
    {0, 0, 0, 0, 0, 0},
    {0, 0, -1, 0, 0, 1},
    {0, 1, 0, 0, -1, 0},
    {0, 2, 2, 0, -2, -2},
    {0, 2, 1, 0, 2, -1},
    {0, -1, 0, 0, 1, 0}
  };
  if (rotation == 0) {
    return;
  }
  int oldmats[6];
  int olddirs[6];
  for (int i = 0; i < 6; i ++) {
    oldmats[i] = mats[i];
    olddirs[i] = dirs[i];
  }
  for (int i = 0; i < 6; i ++) {
    mats[i] = oldmats[mappings[rotation][i]];
    dirs[i] = olddirs[mappings[rotation][i]];
    dirs[i] -= rot_map[rotation][i];
  }
}

void Pixel::render(RenderVecs* allvecs, Collider* collider, int gx, int gy, int gz) {
    if (!render_flag) {
        return;
    }
    
    MemVecs vecs;
    
    gx += px*scale;
    gy += py*scale;
    gz += pz*scale;
    //cout << "render" << gx << ' ' << gy << ' ' << gz << ' '<< scale << endl;
    GLfloat x = gx;
    GLfloat y = gy;
    GLfloat z = gz;
    render_flag = false;
    
    if (value == 0) {
      if (render_index.first > -1) {
        allvecs->del(render_index);
        render_index = pair<int,int>(-1, 0);
      }
      return;
    } else {
      
      
      
      
      //cout << x << ' ' << y << ' ' << z << "rendering call" << endl;
      GLfloat uv_start = 0.0f;// + (1/32.0f*(int)value);
      GLfloat uv_end = uv_start + 1.0f;///32.0f;
      int mat[6];
      for (int i = 0; i < 6; i ++) {
        mat[i] = blocks->blocks[value]->texture[i];
      }
      int dirs[6] {0,0,0,0,0,0};
      
      if (direction != blocks->blocks[value]->default_direction) {
        rotate_to_origin(mat, dirs, blocks->blocks[value]->default_direction);
        rotate_from_origin(mat, dirs, direction);
      }
       
      int minscale = blocks->blocks[value]->minscale;
      
      // /char mat = tex_index;
      /*GLfloat new_uvs[] = {
          0.0f, 1.0f * scale/minscale,
          1.0f * scale/minscale, 1.0f * scale/minscale,
          1.0f * scale/minscale, 0.0f,
          1.0f * scale/minscale, 0.0f,
          0.0f, 0.0f,
          0.0f, 1.0f * scale/minscale
      };*/
      // GLfloat new_uvs[] = {
      //     0.0f, 1.0f,
      //     1.0f, 1.0f,
      //     1.0f, 0.0f,
      //     1.0f, 0.0f,
      //     0.0f, 0.0f,
      //     0.0f, 1.0f
      // };
      //cout << 85 << endl;
      Block* block;
      //cout << 87 << endl;
      
      
      
      
      block = collider->get_global(gx, gy, gz-scale, scale);
      if (block == nullptr or block->is_air(0, 0, 1)) {
          GLfloat new_uvs[] = {
              0.0f, 1.0f,
              1.0f, 1.0f,
              1.0f, 0.0f,
              1.0f, 0.0f,
              0.0f, 0.0f,
              0.0f, 1.0f
          };
          rotate_uv(new_uvs,dirs[5]);
          GLfloat face[] = {
              x + scale, y, z,
              x, y, z,
              x, y + scale, z,
              x, y + scale, z,
              x + scale, y + scale, z,
              x + scale, y, z
          };
          vecs.add_face(face, new_uvs, 0.7f * ( (block!=nullptr) ? block->get_lightlevel(0,0,1) : 1 ), minscale, mat[5]);//0.4f
      }
      
      block = collider->get_global(gx, gy-scale, gz, scale);
      if (block == nullptr or block->is_air(0, 1, 0)) {
          GLfloat new_uvs[] = {
              0.0f, 1.0f,
              1.0f, 1.0f,
              1.0f, 0.0f,
              1.0f, 0.0f,
              0.0f, 0.0f,
              0.0f, 1.0f
          };
          rotate_uv(new_uvs,dirs[4]);
          GLfloat face[] = {
              x, y, z + scale,
              x, y, z,
              x + scale, y, z,
              x + scale, y, z,
              x + scale, y, z + scale,
              x, y, z + scale,
          };
          
          vecs.add_face(face, new_uvs, 0.5f * ( (block!=nullptr) ? block->get_lightlevel(0,1,0) : 1 ), minscale, mat[4]);
      }
      
      block = collider->get_global(gx-scale, gy, gz, scale);
      if (block == nullptr or block->is_air(1, 0, 0)) {
          GLfloat new_uvs[] = {
              0.0f, 1.0f,
              1.0f, 1.0f,
              1.0f, 0.0f,
              1.0f, 0.0f,
              0.0f, 0.0f,
              0.0f, 1.0f
          };
          rotate_uv(new_uvs,dirs[3]);
          GLfloat face[] = {
              x, y, z,
              x, y, z + scale,
              x, y + scale, z + scale,
              x, y + scale, z + scale,
              x, y + scale, z,
              x, y, z
          };
          vecs.add_face(face, new_uvs, 0.7f * ( (block!=nullptr) ? block->get_lightlevel(1,0,0) : 1 ), minscale, mat[3]);
      }
      
      block = collider->get_global(gx, gy, gz+scale, scale);
      if (block == nullptr or block->is_air(0, 0, -1)) {
          GLfloat new_uvs[] = {
              0.0f, 1.0f,
              1.0f, 1.0f,
              1.0f, 0.0f,
              1.0f, 0.0f,
              0.0f, 0.0f,
              0.0f, 1.0f
          };
          rotate_uv(new_uvs,dirs[2]);
          GLfloat face[] = {
              x, y, z + scale,
              x + scale, y, z + scale,
              x + scale, y + scale, z + scale,
              x + scale, y + scale, z + scale,
              x, y + scale, z + scale,
              x, y, z + scale
          };
          vecs.add_face(face, new_uvs, 0.7f * ( (block!=nullptr) ? block->get_lightlevel(0,0,-1) : 1 ), minscale, mat[2]);
      }
      
      block = collider->get_global(gx, gy+scale, gz, scale);
      if (block == nullptr or block->is_air(0, -1, 0)) {
          GLfloat new_uvs[] = {
              0.0f, 1.0f,
              1.0f, 1.0f,
              1.0f, 0.0f,
              1.0f, 0.0f,
              0.0f, 0.0f,
              0.0f, 1.0f
          };
          rotate_uv(new_uvs,dirs[1]);
          GLfloat face[] = {
            
              x, y + scale, z,
              x, y + scale, z + scale,
              x + scale, y + scale, z + scale,
              x + scale, y + scale, z + scale,
              x + scale, y + scale, z,
              x, y + scale, z,
          };
          vecs.add_face(face, new_uvs, 0.9f * ( (block!=nullptr) ? block->get_lightlevel(0,-1,0) : 1 ), minscale, mat[1]);//top
      }
      
      block = collider->get_global(gx+scale, gy, gz, scale);
      if (block == nullptr or block->is_air(-1, 0, 0)) {
          GLfloat new_uvs[] = {
              0.0f, 1.0f,
              1.0f, 1.0f,
              1.0f, 0.0f,
              1.0f, 0.0f,
              0.0f, 0.0f,
              0.0f, 1.0f
          };
          rotate_uv(new_uvs,dirs[0]);
          GLfloat face[] = {
              x + scale, y, z + scale,
              x + scale, y, z,
              x + scale, y + scale, z,
              x + scale, y + scale, z,
              x + scale, y + scale, z + scale,
              x + scale, y, z + scale
          };
          vecs.add_face(face, new_uvs, 0.7f * ( (block!=nullptr) ? block->get_lightlevel(-1,0,0) : 1 ), minscale, mat[0]); //0.4f
      }
    }
    
    if (render_index != pair<int,int>(-1,0)) {
      allvecs->del(render_index);
      render_index = pair<int,int>(-1,0);
    }
    if (vecs.num_verts != 0) {
      render_index = allvecs->add(&vecs);
    }
    
    //cout << render_index << endl;
    //cout << "done rendering" << endl;
    //cout << *num_verts << endl;
    //crash(1);
}

Chunk* Pixel::subdivide() {
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
              char val = value;
              Pixel* pix = new Pixel(x, y, z, val, scale/csize, newchunk, tile);
              newchunk->blocks[x][y][z] = pix;
          }
      }
  }
  if (parent != nullptr) {
      parent->blocks[px][py][pz] = newchunk;
  }
  return newchunk;
}

Chunk* Pixel::subdivide(function<char(ivec3)> gen_func) {
    //render_update();
    if (render_index.first != -1) {
        world->glvecs.del(render_index);
    }
    if (scale == 1) {
        cout << "error: block alreasy at scale 1" << endl;
        return nullptr;
    }
    int gx, gy, gz;
    global_position(&gx, &gy, &gz);
    //cout << "subdivideing " << gx << ' ' << gy << ' ' << gz << ' ' << scale  << endl;
    Chunk * newchunk = new Chunk(px, py, pz, scale, parent);
    for (int x = 0; x < csize; x ++) {
        for (int y = 0; y < csize; y ++) {
            for (int z = 0; z < csize; z ++) {
                char val = gen_func(ivec3(gx+x*scale/csize, gy+y*scale/csize, gz+z*scale/csize));
                Pixel* pix = new Pixel(x, y, z, val, scale/csize, newchunk, tile);
                //cout << gx+x*scale/csize << ' ' << gy+y*scale/csize << ' ' << gz+z*scale/csize << endl;
                Chunk* subdivided = pix->resolve(gen_func);
                if (subdivided == nullptr) {
                   newchunk->blocks[x][y][z] = pix;
                } else {
                  delete pix;
                  newchunk->blocks[x][y][z] = subdivided;
                }
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

Chunk* Pixel::resolve(function<char(ivec3)> gen_func) {
  int gx, gy, gz;
  global_position(&gx, &gy, &gz);
  //cout << gx << ' ' << gy << ' ' << gz << "res" << endl;
  bool shell = true;
  bool full = true;
  char val = gen_func(ivec3(gx, gy, gz));
  for (int ox = 0; ox < scale and shell; ox ++) {
    for (int oy = 0; oy < scale and shell; oy ++) {
      for (int oz = 0; oz < scale and shell; oz ++) {
        char newval = gen_func(ivec3(gx+ox, gy+oy, gz+oz));
        if (newval != val) {
          if (ox == 0 or ox == scale-1 or oy == 0 or oy == scale-1 or oz == 0 or oz == scale-1) {
            shell = false;
          }
          full = false;
        }
      }
    }
  }
  if (shell) {
    if (val == 0 and !full) {
      //cout << "subdivide again" << endl;
      //cout << newchunk->blocks[x][y][z] << '-' << endl;
      return subdivide(gen_func);
    }
  } else {
    //cout << "subdivide again" << endl;
    //cout << newchunk->blocks[x][y][z] << '-' << endl;
    return subdivide(gen_func);
  }
  return nullptr;
}

void Pixel::save_to_file(ostream& of) {
    if (extras == nullptr) {
      if (value != 0 and direction != blocks->blocks[value]->default_direction) {
        of << char(100 + direction);
      }
      of << value;
    } else {
      of << '~' << value;
      extras->save_to_file(of);
    }
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

void Chunk::calculate_lightlevel() {
    for (int x = 0; x < csize; x ++) {
        for (int y = csize-1; y >= 0; y --) {
            for (int z = 0; z < csize; z ++) {
                blocks[x][y][z]->calculate_lightlevel();
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

void Chunk::rotate(int axis, int dir) {
  // order 0,0 0,1 1,0 1,1
  const ivec2 positive[csize*csize] {{1,0},{0,0},{1,1},{0,1}};
  const ivec2 negative[csize*csize] {{0,1},{1,1},{0,0},{1,0}};
  
  for (int i = 0; i < csize; i ++) {
    Block* tmp[4];
    for (int j = 0; j < csize; j ++) {
      for (int k = 0; k < csize; k ++) {
        if (axis == 0) {
          tmp[j*csize+k] = blocks[i][j][k];
        } else if (axis == 1) {
          tmp[j*csize+k] = blocks[k][i][j];
        } else if (axis == 2) {
          tmp[j*csize+k] = blocks[j][k][i];
        }
        tmp[j*csize+k]->rotate(axis, dir);
      }
    }
    for (int j = 0; j < csize; j ++) {
      for (int k = 0; k < csize; k ++) {
        ivec2 newpos;
        if (dir == 1) {
          newpos = positive[j*csize+k];
        } else if (dir == -1) {
          newpos = negative[j*csize+k];
        } else {
          cout << "ERR: dir is not 1 -1 " << endl;
          crash(8237948232222);
        }
        if (axis == 0) {
          tmp[j*csize+k]->px = i;
          tmp[j*csize+k]->py = newpos.x;
          tmp[j*csize+k]->pz = newpos.y;
          blocks[i][newpos.x][newpos.y] = tmp[j*csize+k];
        } else if (axis == 1) {
          tmp[j*csize+k]->px = newpos.y;
          tmp[j*csize+k]->py = i;
          tmp[j*csize+k]->pz = newpos.x;
          blocks[newpos.y][i][newpos.x] = tmp[j*csize+k];
        } else if (axis == 2) {
          tmp[j*csize+k]->px = newpos.x;
          tmp[j*csize+k]->py = newpos.y;
          tmp[j*csize+k]->pz = i;
          blocks[newpos.x][newpos.y][i] = tmp[j*csize+k];
        }
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
                    cout << "blocks.h error" << endl;
                    crash(485934759257949384);
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

void Chunk::render(RenderVecs* vecs, Collider* collider, int gx, int gy, int gz) {
    if (render_flag) {
        render_flag = false;
        for (int x = 0; x < csize; x ++) {
            for (int y = 0; y < csize; y ++) {
                for (int z = 0; z < csize; z ++) {
                    blocks[x][y][z]->render(vecs, collider, gx + px*scale, gy + py*scale, gz + pz*scale);
                }
            }
        }
    }
}

void Chunk::del(bool remove_faces) {
    for (int x = 0; x < csize; x ++) {
        for (int y = 0; y < csize; y ++) {
            for (int z = 0; z < csize; z ++) {
                blocks[x][y][z]->del(remove_faces);
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

void Chunk::save_to_file(ostream& of) {
    of << "{";
    for (int x = 0; x < csize; x ++) {
        for (int y = 0; y < csize; y ++) {
            for (int z = 0; z < csize; z ++) {
                blocks[x][y][z]->save_to_file(of);
            }
        }
    }
    of << "}";
}

#endif
