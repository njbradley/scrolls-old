#ifndef BLOCKS
#define BLOCKS

#include "blocks.h"

#include <cmath>
#include "cross-platform.h"
#include "rendervec.h"
#include "blockdata.h"
#include "tiles.h"
#include "blockgroups.h"
#include "world.h"
#include "entity.h"
#include "mobs.h"
#include "game.h"
#include "glue.h"
#include "generative.h"


const uint8 lightmax = 20;

/*
 *this is the file with classes for storing blocks. the main idea is blocks are stored in power of two containers. Block is the base virtual class,
 *and Pixel and Chunk derive from Block.
 *
 */


#define READ_LOCK ;
#define WRITE_LOCK ;

#ifdef SCROLLS_DEBUG
#define SCROLLS_BLOCK_CHECKS
#endif

#define ISPIXEL (!continues and pixel != nullptr)
#define ISCHUNK continues
#define ISUNDEF (!continues and pixel == nullptr)

#ifdef SCROLLS_BLOCK_CHECKS
#define ASSERT(cond) if (!(cond)) {ERR("ERR: assert failed: " #cond)}
#else
#define ASSERT(cond) ;
#endif

Block::Block(): continues(false), pixel(nullptr) {
  
}

Block::Block(Pixel* pix): continues(false), pixel(pix) {
  pixel->set_block(this);
}

Block::Block(Block* childs): continues(true), children(childs) {
  
}

Block::~Block() {
  if (continues) {
    delete[] children;
  } else if (pixel != nullptr) {
    delete pixel;
  }
}

void Block::set_parent(Block* nparent, Container* nworld, ivec3 ppos, int nscale) {
  parent = nparent;
  world = nworld;
  ASSERT(world != nullptr)
  parentpos = ppos;
  scale = nscale;
  
  if (parent != nullptr) {
    globalpos = parent->globalpos + parentpos * scale;
  } else {
    globalpos = parentpos * scale;
  }
  
  if (continues) {
    for (int i = 0; i < 8; i ++) {
      children[i].set_parent(this, world, ivec3(i/4, i/2%2, i%2), scale / 2);
    }
  }
  ASSERT(parent != this)
  set_render_flag();
  set_light_flag();
}

void Block::set_child(ivec3 pos, Block* block) { ASSERT(ISCHUNK)
  *get(pos) = std::move(*block);
  get(pos)->set_parent(this, world, pos, scale / 2);
}

void Block::set_pixel(Pixel* pix) { ASSERT(ISUNDEF||ISPIXEL)
  pix->set_block(this);
  if (pixel != nullptr) {
    delete pixel;
  }
  pixel = pix;
}

Block* Block::get(ivec3 pos) {
  return children + (pos.x*4 + pos.y * 2 + pos.z);
}

Block* Block::get(int x, int y, int z) {
  return children + (x*4 + y*2 + z);
}

const Block* Block::get(ivec3 pos) const {
  return children + (pos.x*4 + pos.y * 2 + pos.z);
}

const Block* Block::get(int x, int y, int z) const {
  return children + (x*4 + y*2 + z);
}


Block* Block::get_global(int x, int y, int z, int w) {
  return get_global(ivec3(x,y,z), w);
}
  
Block* Block::get_global(ivec3 pos, int w) {
  Block* curblock = this;
  while (SAFEDIV(pos, curblock->scale) != SAFEDIV(curblock->globalpos, curblock->scale)) {
    if (curblock->parent == nullptr) {
      Block* result = world->get_global(pos.x, pos.y, pos.z, w);
      // cout << "give up " << pos << ' ' << curblock->scale << ' ' << curblock->globalpos
      // << ' ' << SAFEDIV(pos, curblock->scale) << ' ' << SAFEDIV(curblock->globalpos, curblock->scale)<< endl;
      return result;
    }
    curblock = curblock->parent;
  }
  
  while (curblock->scale > w and curblock->continues) {
    ivec3 rem = SAFEMOD(pos, curblock->scale) / (curblock->scale / 2);
    curblock = curblock->get(rem);
  }
  return curblock;
}

void Block::set_global(ivec3 pos, int w, int val, int direc, int joints[6]) {
  Block* curblock = this;
  while (SAFEDIV(pos, curblock->scale) != SAFEDIV(curblock->globalpos, curblock->scale)) {
    if (curblock->parent == nullptr) {
      return;// world->get_global(pos.x, pos.y, pos.z, w);
    }
    curblock = curblock->parent;
  }
  
  while (curblock->scale > w) {
    ivec3 rem = SAFEMOD(pos, curblock->scale) / (curblock->scale / 2);
    if (!curblock->continues) {
      curblock->subdivide();
    }
    curblock = curblock->get(rem);
  }
  
  if (curblock->continues) {
    curblock->join();
    curblock->set_pixel(new Pixel(val, direc, joints));
  } else {
    curblock->pixel->set(val, direc, joints);
  }
}

void Block::divide() { ASSERT(!continues)
  if (pixel != nullptr) {
    delete pixel;
  }
  children = new Block[8];
  continues = true;
  for (int i = 0; i < 8; i ++) {
    children[i].set_parent(this, world, ivec3(i/4, i/2%2, i%2), scale / 2);
  }
}

void Block::subdivide() { ASSERT(!continues)
  Pixel pix = *pixel;
  divide();
  for (int i = 0; i < 8; i ++) {
    children[i].set_pixel(new Pixel(pix));
  }
}

void Block::join() { ASSERT(ISCHUNK)
  delete[] children;
  continues = false;
  pixel = nullptr;
}

void Block::to_file(ostream& ofile) const {
  if (continues) {
    ofile << char(0b11000000);
    for (int i = 0; i < 8; i ++) {
      children[i].to_file(ofile);
    }
  } else {
    pixel->to_file(ofile);
  }
}

void Block::from_file(istream& ifile) { ASSERT(ISUNDEF)
  if (ifile.peek() == (0b11000000)) {
    ifile.get();
    divide();
    for (int i = 0; i < 8; i ++) {
      children[i].from_file(ifile);
    }
  } else {
    set_pixel(new Pixel(this, ifile));
  }
}

void Block::set_render_flag() {
  render_flag = true;
  Block* curblock = parent;
  while (curblock != nullptr and !curblock->render_flag) {
    curblock->render_flag = true;
    curblock = curblock->parent;
  }
}

void Block::set_all_render_flags() {
  render_flag = true;
  if (continues) {
    for (int i = 0; i < 8; i ++) {
      children[i].set_all_render_flags();
    }
  }
}

void Block::set_light_flag() {
  light_flag = true;
  Block* curblock = parent;
  while (curblock != nullptr and !curblock->light_flag) {
    curblock->light_flag = true;
    curblock = curblock->parent;
  }
}

void Block::render(RenderVecs* vecs, RenderVecs* transvecs, uint8 faces, bool render_null) {
  if (render_flag) {
    render_flag = false;
    if (continues) {
      for (int i = 0; i < 8; i ++) {
        children[i].render(vecs, transvecs, faces, render_null);
      }
    } else {
      pixel->render(vecs, transvecs, faces, render_null);
    }
  }
}

void Block::lighting_update()  {
  if (light_flag) {
    light_flag = false;
    if (continues) {
      for (int i = 0; i < 8; i ++) {
        children[i].lighting_update();
      }
    } else {
      pixel->lighting_update();
    }
  }
}


BlockIter Block::iter(ivec3 startpos, ivec3 endpos, ivec3 (*iterfunc)(ivec3 pos, ivec3 start, ivec3 end)) {
  if (iterfunc == nullptr) {
    iterfunc = [] (ivec3 pos, ivec3 startpos, ivec3 endpos) {
  		pos.z++;
  		if (pos.z > endpos.z) {
  			pos.z = startpos.z;
  			pos.y ++;
  			if (pos.y > endpos.y) {
  				pos.y = startpos.y;
  				pos.x ++;
  			}
  		}
  		return pos;
  	};
  }
  return BlockIter {this, startpos, endpos, iterfunc};
}

ConstBlockIter Block::const_iter(ivec3 startpos, ivec3 endpos, ivec3 (*iterfunc)(ivec3 pos, ivec3 start, ivec3 end)) const {
  if (iterfunc == nullptr) {
    iterfunc = [] (ivec3 pos, ivec3 startpos, ivec3 endpos) {
  		pos.z++;
  		if (pos.z > endpos.z) {
  			pos.z = startpos.z;
  			pos.y ++;
  			if (pos.y > endpos.y) {
  				pos.y = startpos.y;
  				pos.x ++;
  			}
  		}
  		return pos;
  	};
  }
  return ConstBlockIter {this, startpos, endpos, iterfunc};
}

BlockIter Block::iter_side(ivec3 dir) {
  ivec3 startpos (0,0,0);
  ivec3 endpos (csize-1, csize-1, csize-1);
  startpos += (dir+1)/2 * (csize-1);
  endpos -= (1-dir)/2 * (csize-1);
  return iter(startpos, endpos);
}

BlockIter Block::iter_touching_side(ivec3 dir) {
  ivec3 pos = globalpos;
  pos += dir * scale;
  Block* block = get_global(pos, scale);
  if (block != nullptr) {
    return block->iter_side(-dir);
  } else {
    return BlockIter{.base = nullptr};
  }
}

ConstBlockIter Block::const_iter_side(ivec3 dir) const {
  ivec3 startpos (0,0,0);
  ivec3 endpos (csize-1, csize-1, csize-1);
  startpos += (dir+1)/2 * (csize-1);
  endpos -= (1-dir)/2 * (csize-1);
  return const_iter(startpos, endpos);
}

BlockTouchIter Block::iter_touching() {
  return BlockTouchIter {this, world};
}

vec3 Block::get_position() const {
  return globalpos;
}


int Block::get_sunlight(int dx, int dy, int dz) const {
  int lightlevel = 0;
  int num = 0;
  
  for (const Pixel* pix : const_iter_side(ivec3(dx, dy, dz))) {
    if (pix->parbl->is_air(dx, dy, dz)) {
      lightlevel += pix->sunlight;
      num ++;
    }
  }
  if (num > 0) {
    return lightlevel / num;
  }
  return 0;
}

int Block::get_blocklight(int dx, int dy, int dz) const {
  int lightlevel = 0;
  int num = 0;
  
  for (const Pixel* pix : const_iter_side(ivec3(dx, dy, dz))) {
    if (pix->parbl->is_air(dx, dy, dz)) {
      int light = 0;
      if (pix->blocklight != 0) {
        ivec3 source_dir = dir_array[pix->lightsource];
        if (source_dir.x == dx and source_dir.y == dy and source_dir.z == dz) {
          light = pix->blocklight;
        } else {
          light = pix->blocklight*0.875;
        }
      }
      
      lightlevel += light;
      num ++;
    }
  }
  if (num > 0) {
    return lightlevel / num;
  }
  return 0;
}

bool Block::is_air(int dx, int dy, int dz, char otherval) const {
  for (const Pixel* pix : const_iter_side(ivec3(dx, dy, dz))) {
    bool isair = (pix->value == 0 or (blocks->blocks[pix->value]->transparent and otherval != pix->value));
    if (isair) {
      return true;
    }
  }
  return false;
}

void Block::write_pix_val(ostream& ofile, char type, unsigned int value) {
  ofile << char((type << 6) | ((value%32) << 1) | (value/32 > 0));
  value /= 32;
  while (value > 0) {
    ofile << char(((value%128) << 1) | (value/128 > 0));
    value /= 128;
  }
}

void Block::read_pix_val(istream& ifile, char* type, unsigned int* value) {
  char data = ifile.get();
  *type = (data & 0b11000000) >> 6;
  *value = (data & 0b00111110) >> 1;
  int multiplier = 32;
  while ((data & 0b00000001) and !ifile.eof()) {
    data = ifile.get();
    *value = *value | ((multiplier * (data & 0b11111110)) >> 1);
    multiplier *= 128;
  }
}








/*
Block* Block::raycast(Collider* this_world, double* x, double* y, double* z, double dx, double dy, double dz, double time) {
    //cout << "block raycast(" << *x << ' ' << *y << ' ' << *z << ' ' << dx << ' ' << dy << ' ' << dz << endl;
    if (!continues() and get() != 0 and get() != 7) {
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
    double extra = 0.001;
    
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
*/

  


















Pixel::Pixel(int val, int direct, int njoints[6]):
value(val) {
  if (value != 0) {
    if (blocks->blocks.find(value) == blocks->blocks.end()) {
      cout << "ERR: unknown char code " << int(value) << " (corrupted file?)" << endl;
    }
    if (direct == -1) {
      direction = blocks->blocks[value]->default_direction;
    } else {
      direction = direct;
    }
    if (direction == 255) {
      ERR("bad dir")
    }
  }
  if (njoints != nullptr) {
    for (int i = 0; i < 6; i ++) {
      joints[i] = njoints[i];
    }
  }
  reset_lightlevel();
  
}

Pixel::Pixel(Block* newblock, istream& ifile):
parbl(newblock) {
  bool value_set = false;
  char type;
  unsigned int data;
  direction = 0xff;
  while (!value_set and !ifile.eof()) {
    Block::read_pix_val(ifile, &type, &data);
    if (type == 0b00) {
      value = data;
      value_set = true;
    } else if (type == 0b01) {
      direction = data;
    } else if (type == 0b10) {
      for (int i = 5; i >= 0; i --) {
        joints[i] = data % 32;
        if (joints[i] > 0) {
          joints[i] += 6;
        }
        data /= 32;
      }
    }
  }
  if (direction == 0xff) {
    if (value == 0) {
      direction = 0;
    } else {
      direction = blocks->blocks[value]->default_direction;
    }
  }
  reset_lightlevel();
}

Pixel::~Pixel() {
  erase_render();
}

void Pixel::set_block(Block* nblock) {
  parbl = nblock;
}

void Pixel::rotate(int axis, int dir) {
  WRITE_LOCK;
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
  parbl->set_render_flag();
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

/*
void Pixel::render_face(MemVecs* vecs, GLfloat x, GLfloat y, GLfloat z, Block* blocks[6], int index, ivec3 dir, int uv_dir, int minscale, int mat, bool render_null) {
  GLfloat uvmax = 1.0f;
  Block* block = blocks[index];
  
  
  
  if ((block != nullptr and block->is_air(-dir.x, -dir.y, -dir.z, value)) or (block == nullptr and render_null)) {
    int edges[4] = {0,0,0,0};
    int overlay = 0;
    
    int indexes[6][4] = {
      {5, 1, 2, 4},
      {2, 0, 5, 3},
      {0, 1, 3, 4},
      {2, 1, 5, 4},
      {5, 0, 2, 3},
      {3, 1, 0, 4}
    };
    
    for (int i = 0; i < 4; i ++) {
      int rot_i = (64 + i - uv_dir) % 4;
      if (joints[indexes[index][rot_i]] != 0) {
        edges[i] = joints[indexes[index][rot_i]];
      }
    }
    
    GLfloat face[] = {x,y,z, x,y,z, x,y,z, x,y,z, x,y,z, x,y,z};
    
    GLfloat new_uvs[] = {
        0.0f, uvmax,
        uvmax, uvmax,
        uvmax, 0.0f,
        uvmax, 0.0f,
        0.0f, 0.0f,
        0.0f, uvmax
    };
    rotate_uv(new_uvs,uv_dir);
    
    float faceblocklight = 0, facesunlight = 1;
    if (block != nullptr) {
      faceblocklight = block->get_blocklight(-dir.x, -dir.y, -dir.z) / float(lightmax);
      facesunlight = block->get_sunlight(-dir.x, -dir.y, -dir.z) / float(lightmax);
    }
    
    for (int i = 0; i < 3; i ++) {
      int j,k;
      if (i == 0) {
        k = (i + 1 < 3) ? i+1 : i-2;
        j = (i + 2 < 3) ? i+2 : i-1;
      } else {
        j = (i + 1 < 3) ? i+1 : i-2;
        k = (i + 2 < 3) ? i+2 : i-1;
      }
      if (dir[i] == 1) {
        for (int h = 0; h < 6; h ++) {
          face[h*3+i] += parbl->scale;
        }
      }
      if ((dir[i] == -1 and i > 0) or (i == 0 and dir[i] == 1)) {
        face[0*3+j] += parbl->scale;
        face[4*3+j] += parbl->scale;
        face[5*3+j] += parbl->scale;
        
        face[2*3+k] += parbl->scale;
        face[3*3+k] += parbl->scale;
        face[4*3+k] += parbl->scale;
      } else if ((dir[i] == 1 and i > 0) or (i == 0 and dir[i] == -1)) {
        face[1*3+j] += parbl->scale;
        face[2*3+j] += parbl->scale;
        face[3*3+j] += parbl->scale;
        
        face[2*3+k] += parbl->scale;
        face[3*3+k] += parbl->scale;
        face[4*3+k] += parbl->scale;
      }
    }
    
    if (dir.y == -1) {
      facesunlight *= 0.5f;
    } else if (dir.y == 1) {
      facesunlight *= 0.9f;
    } else {
      facesunlight *= 0.7f;
    }
    
    vecs->add_face(face, new_uvs, facesunlight, faceblocklight, minscale, 0, overlay, edges, mat);
  }
}*/

void Pixel::render(RenderVecs* allvecs, RenderVecs* transvecs, uint8 faces, bool render_null) {
  
  if (render_index.index > -1) {
    lastvecs->del(render_index);
    render_index = RenderIndex::npos;
  }
  
  if (value != 0) {
    ivec3 gpos = parbl->globalpos;
    BlockData* blockdata = blocks->blocks[value];
    RenderData renderdata;
    
    bool exposed = false;
    
    renderdata.pos.loc.pos = gpos;
    renderdata.pos.loc.rot = vec2(0,0);
    renderdata.pos.loc.scale = parbl->scale;
    
    int mat[6];
    for (int i = 0; i < 6; i ++) {
      mat[i] = blockdata->texture[i];
    }
    int dirs[6] {0,0,0,0,0,0};
    
    if (direction != blockdata->default_direction) {
      rotate_to_origin(mat, dirs, blockdata->default_direction);
      rotate_from_origin(mat, dirs, direction);
    }
     
    int minscale = blockdata->minscale;
    int i = 0;
    
    for (int i = 0; i < 6; i ++) {
      ivec3 dir = dir_array[i];
      Block* block = parbl->get_global(gpos + dir_array[i] * parbl->scale, parbl->scale);
      renderdata.type.faces[i].tex = 0;
      if (block != nullptr and block->is_air(-dir.x, -dir.y, -dir.z)) {
        renderdata.type.faces[i].tex = mat[i];
        renderdata.type.faces[i].rot = dirs[i];
        renderdata.type.faces[i].blocklight = block->get_blocklight(-dir.x, -dir.y, -dir.z);
        renderdata.type.faces[i].sunlight = block->get_sunlight(-dir.x, -dir.y, -dir.z);
        exposed = true;
      }
    }
    
    if (exposed) {
      if (blockdata->transparent) {
        render_index = transvecs->add(renderdata);
        lastvecs = transvecs;
      } else {
        render_index = allvecs->add(renderdata);
        lastvecs = allvecs;
      }
    }
  }
}
    
    
  /*
  READ_LOCK;
  
  MemVecs vecs;
  
  int newscale = parbl->scale;
  
  GLfloat x = parbl->globalpos.x;
  GLfloat y = parbl->globalpos.y;
  GLfloat z = parbl->globalpos.z;
  
  if (value == 0) {
    if (render_index.start > -1) {
      lastvecs->del(render_index);
      render_index = RenderIndex::npos;
    }
    return;
  } else {
    
    // if (yield) {
    //   std::this_thread::yield();
    // }
    
    BlockData* blockdata = blocks->blocks[value];
    
    GLfloat uv_start = 0.0f;
    GLfloat uv_end = uv_start + 1.0f;
    int mat[6];
    for (int i = 0; i < 6; i ++) {
      mat[i] = blockdata->texture[i];
    }
    int dirs[6] {0,0,0,0,0,0};
    // if (physicsgroup != nullptr) {
    //   physicsgroup->custom_textures(mat, dirs, ivec3(gx, gy, gz));
    // }
    
    if (direction != blockdata->default_direction) {
      rotate_to_origin(mat, dirs, blockdata->default_direction);
      rotate_from_origin(mat, dirs, direction);
    }
     
    int minscale = blockdata->minscale;
    int i = 0;
    
    Block* blocks[6];
    
    for (ivec3 dir : dir_array) {
      blocks[i] = parbl->get_global(parbl->globalpos+dir*newscale, newscale);
      i ++;
    }
    i = 0;
    for (ivec3 dir : dir_array) {
      if ((faces & (1<<i))) {
        render_face(&vecs, x, y, z, blocks, i, dir, dirs[i], minscale, mat[i], render_null);
      }
      i ++;
    }
    
    if (!render_index.isnull()) {
      lastvecs->del(render_index);
      render_index = RenderIndex::npos;
    }
    if (vecs.num_verts != 0) {
      if (blockdata->transparent) {
        render_index = transvecs->add(&vecs);
        lastvecs = transvecs;
      } else {
        render_index = allvecs->add(&vecs);
        lastvecs = allvecs;
      }
    }
  }
}*/

void Pixel::set(int val, int newdirection, int newjoints[6]) {
  WRITE_LOCK;
  
  if (val == 0) {
    newdirection = 0;
  } else {
    if (newdirection == -1 or !blocks->blocks[val]->rotation_enabled) {
      newdirection = blocks->blocks[val]->default_direction;
    }
  }
  
  bool become_air = value != 0 and val == 0;
  bool become_solid = value == 0 and val != 0;
  value = val;
  
  if (become_air) {
    for (int i = 0; i < 6; i ++) {
      joints[i] = 0;
    }
  }
  
  direction = newdirection;
  
  BlockGroup* oldgroup = nullptr;
  
  if (group != nullptr) {
    oldgroup = group;
    group->del(this);
  }
  
  world->block_update(parbl->globalpos);

  reset_lightlevel();
  render_update();

  Block* block;
  Pixel* lastpix = nullptr;
  int i = 0;
  for (ivec3 unit_dir : dir_array) {
    int inverse_index = (i > 2) ? i-3 : i+3;
    ivec3 dir = unit_dir * parbl->scale;
    block = parbl->get_global(parbl->globalpos + dir, parbl->scale);
    if (block != nullptr) {
      for (Pixel* pix : block->iter_side(-unit_dir)) {
        pix->render_update();
        world->block_update(pix->parbl->globalpos);
        
        if (become_air) {
          pix->joints[inverse_index] = 0;
        } else {
          if (pix->value != 0 and newjoints != nullptr) {
            pix->joints[inverse_index] = newjoints[i];
            joints[i] = newjoints[i];
          } else {
            joints[i] = 0;
          }
        }
        
        if (pix->group != nullptr) {
          int num_touching = std::min(pix->parbl->scale, parbl->scale);
          if (become_air) {
            pix->group->consts[inverse_index] -= num_touching * num_touching;
          } else if (become_solid) {
            pix->group->consts[inverse_index] += num_touching * num_touching;
          }
        }
        
        if (oldgroup != nullptr and pix->group == oldgroup) {
          if (lastpix != nullptr) {
            oldgroup->del(lastpix);
          }
          lastpix = pix;
        }
      }
    }
    i++;
  }
  if (lastpix != nullptr) {
    oldgroup->del(lastpix);
  }
}
  
void Pixel::render_update() {
  parbl->set_render_flag();
  parbl->set_light_flag();
}

void Pixel::random_tick() {
  
}

void Pixel::tick() { // ERR: race condition, render and tick threads race, render renders, tick causes falling
  if (value == 0) {
    if (group != nullptr) {
      group->del(this);
    }
    group = nullptr;
  } else if (group == nullptr) {
    BlockGroup* newgroup = new BlockGroup(world);
    newgroup->spread(this, nullptr, ivec3(0,0,0), 1000);
  }
  // if (group != nullptr and !group->consts[4]) {
    // block_entities.push_back(new FallingBlockEntity(world, group, this));
  // }
}

void Pixel::erase_render() {
  if (render_index.index != -1) {
    lastvecs->del(render_index);
  }
}

void Pixel::lighting_update() {
  READ_LOCK;
  unordered_set<ivec3,ivec3_hash> poses;
  unordered_set<ivec3,ivec3_hash> next_poses;
  calculate_blocklight(poses);
  while (poses.size() > 0) {
    for (ivec3 pos : poses) {
      Block* b = parbl->get_global(pos, 1);
      if (b != nullptr) b->pixel->calculate_blocklight(next_poses);
    }
    poses.swap(next_poses);
    next_poses.clear();
  }
  
  int total = 0;
  int max_size = 0;
  poses.clear();
  next_poses.clear();
  calculate_sunlight(poses);
  while (poses.size() > 0) {
    for (ivec3 pos : poses) {
      Block* b = parbl->get_global(pos, 1);
      if (b != nullptr) {
        b->pixel->calculate_sunlight(next_poses);
      }
    }
    poses.swap(next_poses);
    next_poses.clear();
    if (max_size < poses.size()) {
      max_size = poses.size();
    }
    next_poses.reserve(poses.size());
    total += poses.size();
  }
}

void Pixel::reset_lightlevel() {
  lightsource = 0xff;
  if (value != 0) {
    sunlight = 0;
    blocklight = blocks->blocks[value]->lightlevel;
  } else {
    sunlight = 0;
    blocklight = 0;
  }
  entitylight = 0;
}


void Pixel::calculate_sunlight(unordered_set<ivec3,ivec3_hash>& next_poses) {
  if ((value != 0 and !blocks->blocks[value]->transparent)) {
    return;
  }
  const int decrement = 1;
  int gx, gy, gz;
  gx = parbl->globalpos.x;
  gy = parbl->globalpos.y;
  gz = parbl->globalpos.z;
  const ivec3 dirs[] = {{-1,0,0}, {0,-1,0}, {0,0,-1}, {1,0,0}, {0,1,0}, {0,0,1}};
  ivec3 tilepos = SAFEDIV(parbl->globalpos, World::chunksize);
  int oldsunlight = sunlight;
  sunlight = 0;
  for (ivec3 dir : dirs) {
    int newdec = dir.y == 1 ? 0 : decrement;
    int oppdec = dir.y == -1 ? 0 : decrement;
    Block* block = parbl->get_global(gx+dir.x*parbl->scale, gy+dir.y*parbl->scale, gz+dir.z*parbl->scale, parbl->scale);
    if (block != nullptr) {
      for (Pixel* pix : block->iter_side(-dir)) {
        // if (pix->tile != tile and dir.y == 1 and pix->tile->lightflag) {
          // sunlight = lightmax;
        // } else if (pix->tile != tile and dir.y == -1 and pix->sunlight == lightmax) {
          //sunlight = lightmax;
        // } else
        if (pix->sunlight - newdec*pix->parbl->scale > sunlight and pix->sunlight + oppdec*parbl->scale != oldsunlight) {
          sunlight = pix->sunlight - newdec*pix->parbl->scale;
        }
      }
    } else if (dir.y == 1) {
      sunlight = lightmax;
    }
  }
  
  bool changed = oldsunlight != sunlight;
  for (ivec3 dir : dirs) {
    int newdec = dir.y == -1 ? 0 : decrement;
    Block* block = parbl->get_global(gx+dir.x*parbl->scale, gy+dir.y*parbl->scale, gz+dir.z*parbl->scale, parbl->scale);
    // if (block != nullptr and !(*block->iter().begin())->tile->fully_loaded) {
    //   block = nullptr;
    // }
    if (block != nullptr) {
      for (Pixel* pix : block->iter_side(-dir)) {
        if (pix->sunlight < sunlight - newdec*parbl->scale or (pix->sunlight + newdec*parbl->scale == oldsunlight and sunlight < oldsunlight)) {
          if (tilepos == SAFEDIV(pix->parbl->globalpos, World::chunksize)) {
            next_poses.emplace(pix->parbl->globalpos);
          } else {
            pix->parbl->set_light_flag();
          }
        } else if (tilepos != SAFEDIV(pix->parbl->globalpos, World::chunksize) and dir.y == -1 and pix->sunlight == lightmax) {
          pix->parbl->set_light_flag();
        }
        if (changed) pix->parbl->set_render_flag();
      }
    }
  }
}

void Pixel::calculate_blocklight(unordered_set<ivec3,ivec3_hash>& next_poses) {
  if ((value != 0 and !blocks->blocks[value]->transparent)) {
    return;
  }
  const int decrement = 1;
  int gx, gy, gz;
  gx = parbl->globalpos.x;
  gy = parbl->globalpos.y;
  gz = parbl->globalpos.z;
  ivec3 tilepos = SAFEDIV(parbl->globalpos, World::chunksize);
  const ivec3 dirs[] = {{-1,0,0}, {0,-1,0}, {0,0,-1}, {1,0,0}, {0,1,0}, {0,0,1}};
  int oldblocklight = blocklight;
  blocklight = entitylight;
  int oldsource = lightsource;
  lightsource = -1;
  int index = 0;
  for (ivec3 dir : dirs) {
    Block* block = parbl->get_global(gx+dir.x*parbl->scale, gy+dir.y*parbl->scale, gz+dir.z*parbl->scale, parbl->scale);
    if (block != nullptr) {
      int inverse_index = index<3 ? index + 3 : index - 3;
      for (Pixel* pix : block->iter_side(-dir)) {
        if (pix->blocklight - decrement*pix->parbl->scale > blocklight and pix->lightsource != inverse_index) {
          blocklight = pix->blocklight - decrement*pix->parbl->scale;
          lightsource = index;
        }
      }
    }
    index ++;
  }
  
  bool changed = oldblocklight != blocklight;
  index = 0;
  for (ivec3 dir : dirs) {
    Block* block = parbl->get_global(gx+dir.x*parbl->scale, gy+dir.y*parbl->scale, gz+dir.z*parbl->scale, parbl->scale);
    if (block != nullptr) {
      int inverse_index = index<3 ? index + 3 : index - 3;
      for (Pixel* pix : block->iter_side(-dir)) {
        if (pix->blocklight < blocklight - decrement*parbl->scale or (pix->lightsource == inverse_index and blocklight < oldblocklight)) {
          if (tilepos == SAFEDIV(pix->parbl->globalpos, World::chunksize)) {
            next_poses.emplace(pix->parbl->globalpos);
          } else {
            pix->parbl->set_light_flag();
          }
        }
        if (changed) pix->parbl->set_render_flag();
      }
    }
    index ++;
  }
}

/*
void Pixel::calculate_sunlight(unordered_set<ivec3,ivec3_hash>& next_poses) {
  if ((value != 0 and !blocks->blocks[value]->transparent)) {
    return;
  }
  const int decrement = 1;
  int oldsunlight = sunlight;
  sunlight = 0;
  for (ivec3 dir : dir_array) {
    int newdec = dir.y == 1 ? 0 : decrement;
    int oppdec = dir.y == -1 ? 0 : decrement;
    Block* block = parbl->get_global(parbl->globalpos+dir*parbl->scale, parbl->scale);
    // if (block != nullptr and !(*parbl->iter().begin())->tile->fully_loaded) {
    //   //block = nullptr;
    // }
    if (block != nullptr) {
      for (Pixel* pix : block->iter_side(-dir)) {
        // if (pix->tile != tile and dir.y == 1 and pix->tile->lightflag) {
        //   sunlight = lightmax;
        // } else if (pix->tile != tile and dir.y == -1 and pix->sunlight == lightmax) {
          // sunlight = lightmax;
        // } else
        if (pix->sunlight - newdec*pix->parbl->scale > sunlight and pix->sunlight + oppdec*parbl->scale != oldsunlight) {
          sunlight = pix->sunlight - newdec*pix->parbl->scale;
        }
      }
    } else if (dir.y == 1) {
      sunlight = lightmax;
    }
  }
  
  ivec3 tilepos = SAFEMOD(parbl->globalpos, World::chunksize);
  bool changed = oldsunlight != sunlight;
  for (ivec3 dir : dir_array) {
    int newdec = dir.y == -1 ? 0 : decrement;
    Block* block = parbl->get_global(parbl->globalpos+dir*parbl->scale, parbl->scale);
    if (block != nullptr) {
      for (Pixel* pix : block->iter_side(-dir)) {
        if (pix->sunlight < sunlight - newdec*parbl->scale or (pix->sunlight + newdec*parbl->scale == oldsunlight and sunlight < oldsunlight)) {
          if (SAFEMOD(pix->parbl->globalpos, World::chunksize) == tilepos) {
            next_poses.emplace(pix->parbl->globalpos);
          } else {
            pix->parbl->set_light_flag();
          }
        } else if (SAFEMOD(pix->parbl->globalpos, World::chunksize) == tilepos
         and dir.y == -1 and pix->sunlight == lightmax) {
          pix->parbl->set_light_flag();
        }
        if (changed) pix->parbl->set_render_flag();
      }
    }
  }
}

void Pixel::calculate_blocklight(unordered_set<ivec3,ivec3_hash>& next_poses) {
  if ((value != 0 and !blocks->blocks[value]->transparent)) {
    return;
  }
  const int decrement = 1;
  int oldblocklight = blocklight;
  blocklight = entitylight;
  int oldsource = lightsource;
  lightsource = 0xff;
  int index = 0;
  for (ivec3 dir : dir_array) {
    Block* block = parbl->get_global(parbl->globalpos+dir*parbl->scale, parbl->scale);
    if (block != nullptr) {
      int inverse_index = index<3 ? index + 3 : index - 3;
      for (Pixel* pix : block->iter_side(-dir)) {
        if (pix->blocklight - decrement*pix->parbl->scale > blocklight and pix->lightsource != inverse_index) {
          blocklight = pix->blocklight - decrement*pix->parbl->scale;
          lightsource = index;
        }
      }
    }
    index ++;
  }
  
  bool changed = oldblocklight != blocklight;
  ivec3 tilepos = SAFEMOD(parbl->globalpos, World::chunksize);
  index = 0;
  for (ivec3 dir : dir_array) {
    Block* block = parbl->get_global(parbl->globalpos+dir*parbl->scale, parbl->scale);
    if (block != nullptr) {
      int inverse_index = index<3 ? index + 3 : index - 3;
      for (Pixel* pix : block->iter_side(-dir)) {
        if (pix->blocklight < blocklight - decrement*parbl->scale or (pix->lightsource == inverse_index and blocklight < oldblocklight)) {
          if (SAFEMOD(pix->parbl->globalpos, World::chunksize) == tilepos) {
            next_poses.emplace(pix->parbl->globalpos);
          } else {
            pix->parbl->set_light_flag();
          }
        }
        if (changed) pix->parbl->set_render_flag();
      }
    }
    index ++;
  }
}
*/
bool Pixel::is_air(int x, int y, int z) {
  return value == 0;
}

BlockGroupIter Pixel::iter_group() {
  if (group == nullptr) {
    return {this, parbl->world, [] (Pixel* base, Pixel* pix) {
      return pix->value == base->value;
    }};
  } else {
    return {this, parbl->world, [] (Pixel* base, Pixel* pix) {
      return pix->group == base->group;
    }};
  }
}

BlockGroupIter Pixel::iter_group(bool (*func)(Pixel*,Pixel*)) {
  return {this, parbl->world, func};
}

BlockGroupIter Pixel::iter_group(Collider* world, bool (*func)(Pixel*,Pixel*)) {
  if (func == nullptr) {
    if (group == nullptr) {
      func = [] (Pixel* base, Pixel* pix) {
        return pix->value == base->value;
      };
    } else {
      func = [] (Pixel* base, Pixel* pix) {
        return pix->group == base->group;
      };
    }
  }
  return {this, world, func};
}

void Pixel::to_file(ostream& of) {
  int jointval = 0;
  for (int i = 0; i < 6; i ++) {
    jointval *= 32;
    if (joints[i] > 6) {
      jointval += joints[i] - 6;
    }
  }
  if (jointval != 0) {
    Block::write_pix_val(of, 0b10, jointval);
  }
  if (value != 0 and direction != blocks->blocks[value]->default_direction) {
    Block::write_pix_val(of, 0b01, direction);
  }
  Block::write_pix_val(of, 0b00, value);
}





BlockContainer::BlockContainer(Block* b): block(b), allocated(false) {
	block->set_parent(nullptr, this, ivec3(0,0,0), b->scale);
}

BlockContainer::BlockContainer(int scale): allocated(true) {
  block = new Block();
	block->set_parent(nullptr, this, ivec3(0,0,0), scale);
}

BlockContainer::~BlockContainer() {
  if (allocated) {
    delete block;
  }
}

vec3 BlockContainer::get_position() const {
	return block->get_position();
}

Block* BlockContainer::get_global(int x, int y, int z, int size) {
	if (x >= 0 and y >= 0 and z >= 0 and x < block->scale and y < block->scale and z < block->scale) {
		return block->get_global(x, y, z, size);
	} else {
		return nullptr;
	}
}

void BlockContainer::set(ivec4 pos, char val, int direction, int newjoints[6]) {
  // Block* testblock;
  // while ((testblock = get_global(pos.x, pos.y, pos.z, pos.w)) == nullptr) {
  //   Pixel* pix = new Pixel(0, 0, 0, 0, block->scale * csize, nullptr, nullptr);
  //   Chunk* chunk = pix->subdivide();
  //   chunk->blocks[0][0][0] = block;
  //   block->parent = chunk;
  //   block = chunk;
  //   pix->del(true);
  //   delete pix;
  // }
  
  block->set_global(pos, pos.w, val, direction, newjoints);
}






Pixel* BlockIter::iterator::operator*() {
  return pix;
}


BlockIter::iterator BlockIter::iterator::operator++() {
  Block* block = pix->parbl;
  while (block->parent != parent->base->parent and block->parentpos == parent->end_pos) {
    block = block->parent;
  }
  if (block->parent == parent->base->parent) {
    pix = nullptr;
    return *this;
  }
  ivec3 pos = block->parentpos;
  pos = parent->increment_func(pos, parent->start_pos, parent->end_pos);
  block = block->parent->get(pos);
  while (block->continues) {
    block = block->get(parent->start_pos);
  }
  pix = block->pixel;
  return *this;
}

BlockIter::iterator BlockIter::begin() {
  if (base == nullptr) {
    return end();
  }
  Block* block = base;
  while (block->continues) {
    block = block->get(start_pos);
  }
  return {this, block->pixel};
}

BlockIter::iterator BlockIter::end() {
  return {this, nullptr};
}

bool operator!=(const BlockIter::iterator& iter1, const BlockIter::iterator& iter2) {
  return iter1.parent != iter2.parent or iter1.pix != iter2.pix;
}





Pixel* BlockTouchIter::iterator::operator*() {
  return *iter;
}

BlockTouchIter::iterator BlockTouchIter::iterator::operator++() {
  ++iter;
  while (!(iter != parent->blockiter.end())) {
    dir_index ++;
    if (dir_index >= 6) {
      dir_index = -1;
      return *this;
    }
    ivec3 pos = parent->base->globalpos;
    int scale = parent->base->scale;
    ivec3 dir = dir_array[dir_index];
    Block* block = parent->base->get_global(pos+dir*scale, scale);
    if (block != nullptr) {
      parent->blockiter = block->iter_side(-dir);
      iter = parent->blockiter.begin();
    }
  }
  return *this;
}

BlockTouchIter::iterator BlockTouchIter::begin() {
  int dir_index = -1;
  Block* block;
  ivec3 dir;
  ivec3 pos = base->globalpos;
  int scale = base->scale;
  do {
    dir_index ++;
    if (dir_index >= 6) {
      return end();
    }
    dir = dir_array[dir_index];
    block = base->get_global(pos+dir*scale, scale);
  } while (block == nullptr);
  blockiter = block->iter_side(-dir);
  return {this, dir_index, blockiter.begin()};
}

BlockTouchIter::iterator BlockTouchIter::end() {
  BlockTouchIter::iterator iter;
  iter.parent = this;
  iter.dir_index = -1;
  return iter;
}

bool operator!=(const BlockTouchIter::iterator& iter1, const BlockTouchIter::iterator& iter2) {
  if (iter1.parent == iter2.parent and iter1.dir_index == -1 and iter2.dir_index == -1) {
    return false;
  }
  return iter1.parent != iter2.parent or iter1.iter != iter2.iter;
}



BlockGroupIter::BlockGroupIter(Pixel* newbase, Collider* nworld, bool (*func)(Pixel* base, Pixel* pix)):
base(newbase), world(nworld), includefunc(func) {
  unordered_set<Pixel*> new_pixels;
  unordered_set<Pixel*> last_pixels;
  
  last_pixels.emplace(base);
  
  while (last_pixels.size() > 0) {
    for (Pixel* pix : last_pixels) {
      pixels.emplace(pix);
    }
    
    for (Pixel* pix : last_pixels) {
      for (Pixel* sidepix : pix->parbl->iter_touching()) {
        if (includefunc(base, sidepix) and pixels.count(sidepix) == 0 and new_pixels.count(sidepix) == 0) {
          new_pixels.emplace(sidepix);
        }
      }
    }
    
    last_pixels.swap(new_pixels);
    new_pixels.clear();
  }
}



unordered_set<Pixel*>::iterator BlockGroupIter::begin() {
  return pixels.begin();
}

unordered_set<Pixel*>::iterator BlockGroupIter::end() {
  return pixels.end();
}

void BlockGroupIter::swap(unordered_set<Pixel*>& other) {
  pixels.swap(other);
}



const Pixel* ConstBlockIter::iterator::operator*() {
  return pix;
}

ConstBlockIter::iterator ConstBlockIter::iterator::operator++() {
  const Block* block = pix->parbl;
  while (block->parent != parent->base->parent and block->parentpos == parent->end_pos) {
    block = block->parent;
  }
  if (block->parent == parent->base->parent) {
    pix = nullptr;
    return *this;
  }
  ivec3 pos = block->parentpos;
  pos = parent->increment_func(pos, parent->start_pos, parent->end_pos);
  block = block->parent->get(pos);
  while (block->continues) {
    block = block->get(parent->start_pos);
  }
  pix = block->pixel;
  return *this;
}

ConstBlockIter::iterator ConstBlockIter::begin() {
  const Block* block = base;
  while (block->continues) {
    block = block->get(start_pos);
  }
  return {this, block->pixel};
}

ConstBlockIter::iterator ConstBlockIter::end() {
  return {this, nullptr};
}

bool operator!=(const ConstBlockIter::iterator& iter1, const ConstBlockIter::iterator& iter2) {
  return iter1.parent != iter2.parent or iter1.pix != iter2.pix;
}

#endif
