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
#include "game.h"
#include "glue.h"
#include "generative.h"
#include "blockiter.h"

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

#ifdef SCROLLS_BLOCK_CHECKS
#define ASSERT(cond) if (!(cond)) {ERR("ERR: assert failed: " #cond)}
#else
#define ASSERT(cond) ;
#endif


int children_begin(Block** children) {
  for (int i = 0; i < csize3; i ++) {
    if (children[i] != nullptr) {
      return i;
    }
  }
  return csize3;
}

int children_inc(Block** children, int i) {
  i ++;
  while (i < csize3 and children[i] == nullptr) i ++;
  return i;
}
// Shorthand for:
// for (int i = 0; i < csize3; i ++) {
//    if (children[i] != nullptr) {
//      .....
#define CHILDREN_LOOP(varname) int varname = children_begin(children); varname < csize3; varname = children_inc(children, varname)

#define FREECHILDREN_LOOP(varname) FreeBlock* varname = freechild; varname != nullptr; varname = varname->freechild

Block::Block(): continues(false), pixel(nullptr) {
  // for (int i = 0; i < csize3; i ++) {
  //   children[i] = nullptr;
  // }
}

Block::Block(Block* childs[8]): continues(true) {
  for (int i = 0; i < csize3; i ++) {
    children[i] = childs[i];
  }
}

Block::Block(Pixel* pix): continues(false), pixel(pix) {
  pixel->set_block(this);
}

Block::~Block() {
  if (continues) {
    for (CHILDREN_LOOP(i)) {
      delete children[i];
    }
  } else if (pixel != nullptr) {
    if (game->debugblock == pixel) {
      game->debugblock = nullptr;
    }
    delete pixel;
  }
  if (freechild != nullptr) {
    delete freechild;
  }
}


void Block::lock() {
  locked = true;
}

void Block::unlock() {
  locked = false;
}

void Block::set_parent(Container* nworld, ivec3 ppos, int nscale) {
  set_parent(nullptr, nworld, nullptr, ppos, nscale);
}

void Block::set_parent(Block* nparent, Container* nworld, FreeBlock* freecont, ivec3 ppos, int nscale) {
  parent = nparent;
  world = nworld;
  freecontainer = freecont;
  parentpos = ppos;
  scale = nscale;
  
  if (parent != nullptr) {
    globalpos = parent->globalpos + parentpos * scale;
  } else {
    globalpos = parentpos * scale;
  }
  
  if (continues) {
    for (CHILDREN_LOOP(i)) {
      children[i]->set_parent(this, world, freecontainer, posof(i), scale / csize);
    }
  }
  set_flag(RENDER_FLAG | LIGHT_FLAG);
}

void Block::set_child(ivec3 pos, Block* block) { ASSERT(ISCHUNK)
  Block* old = get(pos);
  if (old != nullptr) {
    delete old;
  }
  children[indexof(pos)] = block;
  block->set_parent(this, world, freecontainer, pos, scale / csize);
}

Block* Block::swap_child(ivec3 pos, Block* block) { ASSERT(ISCHUNK)
  Block* old = get(pos);
  children[indexof(pos)] = block;
  block->set_parent(this, world, freecontainer, pos, scale / csize);
  return old;
}

void Block::set_pixel(Pixel* pix) {
  pix->set_block(this);
  if (pixel != nullptr) {
    if (game->debugblock == pixel) {
      game->debugblock = nullptr;
    }
    delete pixel;
  }
  pixel = pix;
  set_flag(RENDER_FLAG | LIGHT_FLAG);
}

Pixel* Block::swap_pixel(Pixel* pix) { ASSERT(ISPIXEL)
  pix->set_block(this);
  Pixel* old = pixel;
  pixel = pix;
  set_flag(RENDER_FLAG | LIGHT_FLAG);
  return old;
}


void Block::add_freechild(FreeBlock* newfree) {
  FreeBlock** freedest = &freechild;
  while (*freedest != nullptr) {
    freedest = &(*freedest)->freechild;
  }
  
  *freedest = newfree;
  newfree->set_parent(this, world, ivec3(0,0,0), scale/2);
}

void Block::remove_freechild(FreeBlock* newfree) {
  FreeBlock** freedest = &freechild;
  while (*freedest != nullptr and *freedest != newfree) {
    freedest = &(*freedest)->freechild;
  }
  if (*freedest != nullptr) {
    *freedest = (*freedest)->freechild;
  }
}


Block* Block::get(ivec3 pos) {
  return children[pos.x*csize*csize + pos.y*csize + pos.z];
}

Block* Block::get(int x, int y, int z) {
  return children[x*csize*csize + y*csize + z];
}

const Block* Block::get(ivec3 pos) const {
  return children[pos.x*csize*csize + pos.y*csize + pos.z];
}

const Block* Block::get(int x, int y, int z) const {
  return children[x*csize*csize + y*csize + z];
}

ivec3 Block::posof(int index) const {
  return ivec3(index/(csize*csize), index/csize%csize, index%csize);
}

int Block::indexof(ivec3 pos) const {
  return pos.x*csize*csize + pos.y*csize + pos.z;
}

int Block::indexof(int x, int y, int z) const {
  return x*csize*csize + y*csize + z;
}

quat Block::getrotation() const {
  quat rot (1,0,0,0);
  FreeBlock* free = freecontainer;
  while (free != nullptr) {
    rot = rot * free->box.rotation;
    free = free->highparent->freecontainer;
  }
  return rot;
}

vec3 Block::fglobalpos() const {
  vec3 pos = globalpos;
  FreeBlock* free = freecontainer;
  while (free != nullptr) {
    pos = free->box.transform_out(pos);
    free = free->highparent->freecontainer;
  }
  return pos;
}

Hitbox Block::local_hitbox() {
  return Hitbox(vec3(0,0,0), globalpos, globalpos + scale);
}

Hitbox Block::local_freebox() {
  return Hitbox(vec3(0,0,0), vec3(globalpos) - scale/2.0f, vec3(globalpos) + scale*3/2.0f);
}

Hitbox Block::hitbox() {
  if (freecontainer == nullptr) {
    return local_hitbox();
  } else {
    return freecontainer->box.transform_out(local_hitbox());
  }
}

Hitbox Block::freebox() {
  if (freecontainer == nullptr) {
    return local_freebox();
  } else {
    return freecontainer->box.transform_out(local_freebox());
  }
}

Block* Block::get_global(int x, int y, int z, int w) {
  return get_global(ivec3(x,y,z), w);
}
  
Block* Block::get_global(ivec3 pos, int w) {
  Block* curblock = this;
  while (SAFEDIV(pos, curblock->scale) != SAFEDIV(curblock->globalpos, curblock->scale) or curblock->scale < w) {
    if (curblock->parent == nullptr) {
      if (curblock->freecontainer != nullptr) {
        pos = SAFEFLOOR3(curblock->freecontainer->box.transform_out(pos));
        curblock = curblock->freecontainer->highparent;
      } else {
        Block* result = world->get_global(pos, w);
        return result;
      }
    } else {
      curblock = curblock->parent;
    }
  }
  
  while (curblock->scale > w and curblock->continues) {
    ivec3 rem = SAFEMOD(pos, curblock->scale) / (curblock->scale / csize);
    curblock = curblock->get(rem);
  }
  return curblock;
}

Block* Block::get_global(vec3 pos, int w, vec3 dir) {
  Block* curblock = this;
  const float tiny = 0.001f;
  while (SAFEFLOOR3((pos + dir * tiny) / float(curblock->scale)) != SAFEDIV(curblock->globalpos, curblock->scale)) {
    if (curblock->parent == nullptr) {
      if (curblock->freecontainer != nullptr) {
        pos = curblock->freecontainer->box.transform_out(pos);
        dir = curblock->freecontainer->box.transform_out_dir(dir);
        curblock = curblock->freecontainer->highparent;
      } else {
        Block* result = world->get_global(ivec3(pos+dir*tiny), w);
        return result;
      }
    } else {
      curblock = curblock->parent;
    }
  }
  
  while (curblock->scale > w and curblock->continues) {
    vec3 rem = pos + dir * tiny - ((pos + dir * tiny) / float(curblock->scale)) * float(curblock->scale);
    curblock = curblock->get(SAFEFLOOR3(rem));
  }
  
  // for (int i = 0; i < csize3 and curblock->freechilds[i] != nullptr; i ++) {
  //   FreeBlock* freechild = curblock->freechilds[i]->freecontainer;
  //   Block* block = freechild->get_local(freechild->box.transform_into(pos), w, freechild->box.transform_into_dir(dir));
  //   if (block != nullptr) {
  //     return block;
  //   }
  // }
  
  return curblock;
}

Block* Block::get_local(ivec3 pos, int w) {
  Block* curblock = this;
  while (SAFEDIV(pos, curblock->scale) != SAFEDIV(curblock->globalpos, curblock->scale)) {
    if (curblock->parent == nullptr) {
      if (curblock->freecontainer != nullptr) {
        return nullptr;
      } else {
        Block* result = world->get_global(pos.x, pos.y, pos.z, w);
        return result;
      }
    } else {
      curblock = curblock->parent;
    }
  }
  
  while (curblock->scale > w and curblock->continues) {
    ivec3 rem = SAFEMOD(pos, curblock->scale) / (curblock->scale / csize);
    curblock = curblock->get(rem);
  }
  return curblock;
}

Block* Block::get_local(vec3 pos, int w, vec3 dir) {
  Block* curblock = this;
  const float tiny = 0.001f;
  while (SAFEFLOOR3((pos + dir * tiny) / float(curblock->scale)) != SAFEDIV(curblock->globalpos, curblock->scale)) {
    if (curblock->parent == nullptr) {
      return nullptr;
    }
    curblock = curblock->parent;
  }
  
  while (curblock->scale > w and curblock->continues) {
    vec3 rem = pos + dir * tiny - ((pos + dir * tiny) / float(curblock->scale)) * float(curblock->scale);
    curblock = curblock->get(SAFEFLOOR3(rem));
  }
  return curblock;
}

void Block::set_global(ivec3 pos, int w, Blocktype val, int direc, int joints[6]) {
  
  Block* curblock = this;
  while (SAFEDIV(pos, curblock->scale) != SAFEDIV(curblock->globalpos, curblock->scale)) {
    if (curblock->parent == nullptr) {
      if (curblock->freecontainer != nullptr) {
        ivec3 expand_dir = glm::sign(SAFEDIV(pos, curblock->scale) - SAFEDIV(curblock->globalpos, curblock->scale));
        ivec3 expand_off = (-expand_dir + 1) / 2;
        pos += expand_off * curblock->scale;
        curblock->freecontainer->expand(expand_dir);
        curblock = curblock->get(0,0,0);
      } else {
        world->set_global(pos, w, val, direc, joints);
        return;
      }
    }
    curblock = curblock->parent;
  }
  
  while (curblock->scale > w) {
    ivec3 rem = SAFEMOD(pos, curblock->scale) / (curblock->scale / csize);
    if (!curblock->continues) {
      cout << "subdividing" << endl;
      curblock->subdivide();
    }
    curblock = curblock->get(rem);
  }
  
  if (curblock->continues) {
    cout << "joining" << endl;
    curblock->join();
    curblock->set_pixel(new Pixel(val, direc, joints));
  } else {
    cout << "setting" << endl;
    curblock->pixel->set(val, direc, joints);
  }
}

void Block::divide() { ASSERT(!continues)
  if (pixel != nullptr) {
    if (game->debugblock == pixel) {
      game->debugblock = nullptr;
    }
    delete pixel;
  }
  continues = true;
  for (int i = 0; i < csize3; i ++) {
    children[i] = nullptr;
  }
}

void Block::subdivide() { ASSERT(!continues)
  lock();
  Pixel* pix = pixel;
  divide();
  for (int i = 0; i < csize3; i ++) {
    set_child(posof(i), new Block(new Pixel(pix->value, pix->direction)));
  }
  unlock();
  set_flag(RENDER_FLAG | LIGHT_FLAG);
}

void Block::join() { ASSERT(ISCHUNK)
  for (CHILDREN_LOOP(i)) {
    delete children[i];
  }
  continues = false;
  pixel = nullptr;
}

void Block::to_file(ostream& ofile) const {
  if (continues) {
    ofile << char(0b11000000);
    for (int i = 0; i < csize3; i ++) {
      if (children[i] != nullptr) {
        children[i]->to_file(ofile);
      } else {
        ofile << char(0b11111111);
      }
    }
  } else {
    pixel->to_file(ofile);
  }
}

void Block::from_file(istream& ifile) {
  if (ifile.peek() == (0b11000000)) {
    ifile.get();
    divide();
    for (int i = 0; i < csize3; i ++) {
      if (ifile.peek() == (0b11111111)) {
        ifile.get();
        children[i] = nullptr;
      } else {
        set_child(posof(i), new Block(ifile));
      }
    }
  } else {
    set_pixel(new Pixel(this, ifile));
  }
}

Block::Block(istream& ifile): continues(false), pixel(nullptr) {
  from_file(ifile);
}

void Block::set_flag(uint8 flag) {
  Block* curblock = this;
  while (curblock != nullptr and !(curblock->flags & flag) and !curblock->locked) {
    dkout << " flag setting " << curblock << ' ' << curblock->globalpos << ' ' << curblock->scale << ' ' << int(curblock->flags);
    curblock->flags |= flag;
    dkout << ' ' << int(curblock->flags) << endl;
    if (curblock->parent == nullptr and curblock->freecontainer != nullptr) {
      curblock = curblock->freecontainer->highparent;
    } else {
      curblock = curblock->parent;
    }
  }
}

void Block::set_all_flags(uint8 flag) {
  flags |= flag;
  if (continues) {
    for (CHILDREN_LOOP(i)) {
      children[i]->set_all_flags(flags);
    }
  }
  for (FREECHILDREN_LOOP(free)) {
    free->set_all_flags(flags);
  }
}

void Block::timestep(float deltatime) {
  if (freecontainer != nullptr) {
    freecontainer->timestep(deltatime);
    return;
  }
  
  if (freechild != nullptr) {
    freechild->timestep(deltatime);
  }
  
  if (continues) {
    for (int i = 0; i < csize3; i ++) {
      children[i]->timestep(deltatime);
    }
  }
}



void Block::render(RenderVecs* vecs, RenderVecs* transvecs, uint8 faces, bool render_null) {
  if (flags & RENDER_FLAG and !locked) {
    dkout << "RENDERING " << int(flags) << ' ' << this << ' ' << globalpos << ' ' << scale << endl;
    flags &= ~RENDER_FLAG;
    dkout << "  NOW " << int(flags) << endl;
    if (continues) {
      for (CHILDREN_LOOP(i)) {
        children[i]->render(vecs, transvecs, faces, render_null);
      }
    } else {
      pixel->render(vecs, transvecs, faces, render_null);
    }
    for (FREECHILDREN_LOOP(free)) {
      free->render(vecs, transvecs, faces, render_null);
    }
  }
}

void Block::lighting_update()  {
  if (flags & LIGHT_FLAG and !locked) {
    flags &= ~LIGHT_FLAG;
    if (continues) {
      for (CHILDREN_LOOP(i)) {
        children[i]->lighting_update();
      }
    } else {
      pixel->lighting_update();
      for (FREECHILDREN_LOOP(free)) {
        free->lighting_update();
      }
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
      // cout << '-' << pos << ' ' << startpos << endl;
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

BlockTouchSideIter Block::iter_touching_side(ivec3 dir) {
  return BlockTouchSideIter(this, dir);
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


int Block::get_sunlight(ivec3 dir) {
  int lightlevel = 0;
  int num = 0;
  
  for (Pixel* pix : iter_touching_side(dir)) {
    if (pix->value == 0 or pix->value == 7) {
      lightlevel += pix->sunlight;
      num ++;
    }
  }
  if (num > 0) {
    return lightlevel / num;
  }
  return 0;
}

int Block::get_blocklight(ivec3 dir) {
  int lightlevel = 0;
  int num = 0;
  
  for (Pixel* pix : iter_touching_side(dir)) {
    if (pix->value == 0 or pix->value == 7) {
      int light = 0;
      if (pix->blocklight != 0) {
        ivec3 source_dir = dir_array[pix->lightsource];
        if (source_dir.x == -dir.x and source_dir.y == -dir.y and source_dir.z == -dir.z) {
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

bool Block::is_air(ivec3 dir, char otherval) {
  dkout << "Block isair " << endl;
  for (const Pixel* pix : iter_touching_side(dir)) {
    dkout << pix->parbl << ' ' << pix->parbl->globalpos << ' ' << pix->parbl->scale << ' ' << pix->parbl->freecontainer << endl;
    bool isair = (pix->value == 0 or (blocks->blocks[pix->value]->transparent and otherval != pix->value));
    if (isair) {
      return true;
    }
  }
  return false;
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





Block* Block::raycast(vec3* pos, vec3 dir, double timeleft) { ASSERT(ISPIXEL)
  Block* curblock = this;
  
  
  
  
  
  
  
  
  
  // cout << "Raycast " << *pos << ' ' << dir << ' ' << timeleft << endl;
  
  vec3 axis = glm::cross(vec3(1,0,0), dir);
  axis /= glm::length(axis);
  
  quat rot = glm::angleAxis(float(acos(glm::dot(vec3(1,0,0), dir))), axis);
  
  double starttime = timeleft;
  vec3 startpos = *pos;
  //
  // Hitbox linebox (*pos, vec3(0,0,0), vec3(timeleft, 0, 0), rot);
  //
  // return curblock;
  
  while (curblock != nullptr and (curblock->pixel->value == 0 or curblock->pixel->value == 7)) {
    /*
    Hitbox boxhit;
    Block* parblock = curblock;
    while (parblock != nullptr) {
      for (int x = -1; x < 2; x ++) {
        for (int y = -1; y < 2; y ++) {
          for (int z = -1; z < 2; z ++) {
            ivec3 off (x,y,z);
            Block* block = parblock->get_global(parblock->globalpos + off * parblock->scale, parblock->scale);
    */
      
    
    ivec3 gpos = curblock->globalpos;
    vec3 relpos = *pos - vec3(gpos);
    float mintime = 999999;
    for (int i = 0; i < 3; i ++) {
      float time = 999999;
      if (dir[i] < 0) {
        time = -relpos[i] / dir[i];
      } else if (dir[i] > 0) {
        time = (curblock->scale - relpos[i]) / dir[i];
      }
      if (time < mintime) {
        mintime = time;
      }
    }
    *pos += dir * (mintime + 0.001f);
    timeleft -= mintime;
    if (timeleft < 0) {
      curblock = nullptr;
    } else {
      curblock = curblock->get_local(SAFEFLOOR3(*pos), 1);
    }
  }
  
  Hitbox linebox (startpos, vec3(0,0,0), vec3(starttime, 0, 0), rot);
  cout << linebox << ' ' << linebox.boundingbox() << endl;
  return curblock;
  FreeBlockIter freeiter(world, linebox.boundingbox());
  
  double best_time = timeleft;
  
  for (int i = 0; i < freeiter.num_bases; i ++) {
    Block* block = freeiter.bases[i];
    cout << block << ' ' << block->globalpos << ' ' << block->scale << ' ' << endl;
    
    Block* parblock = block;
    cout << "STARTING " << block << endl;
    while (parblock != nullptr) {
      cout << parblock << ' ' << parblock->globalpos << ' ' << parblock->scale << endl;
      for (int x = -1; x < 2; x ++) {
        for (int y = -1; y < 2; y ++) {
          for (int z = -1; z < 2; z ++) {
            ivec3 off (x,y,z);
            Block* offblock = parblock->get_global(parblock->globalpos + off * parblock->scale, parblock->scale);
            if (offblock != nullptr and offblock->scale == parblock->scale) {
              cout << ' ' << offblock << offblock->globalpos << ' ' << offblock->scale << endl;
              for (FreeBlock* freeblock = offblock->freechild; freeblock != nullptr; freeblock = freeblock->freechild) {
                cout << "freeblock " << freeblock << endl;
                if (freeblock->box.collide(linebox, 0)) {
                  cout << " going into free block" << endl;
                  vec3 start = freeblock->box.transform_in(*pos);
                  vec3 newdir = freeblock->box.transform_in(dir);
                  
                  
                  // cout << start << ' ' << newdir << endl;
                  float dt = 0;
                  float maxdt = -1;
                  bool ray_valid = true;
                  for (int axis = 0; axis < 3 and ray_valid; axis ++) {
                    float mintime;
                    float maxtime;
                    
                    if (start[axis] < 0) {
                      mintime = (-start[axis]) / newdir[axis];
                      maxtime = (freeblock->scale - start[axis]) / newdir[axis];
                    } else if (start[axis] > freeblock->scale) {
                      mintime = (freeblock->scale - start[axis]) / newdir[axis];
                      maxtime = (-start[axis]) / newdir[axis];
                    } else {
                      mintime = 0;
                      if (newdir[axis] < 0) {
                        maxtime = (-start[axis]) / newdir[axis];
                      } else {
                        maxtime = (freeblock->scale - start[axis]) / newdir[axis];
                      }
                    }
                    // cout << mintime << ' ' << maxtime << endl;
                    if (maxdt == -1) {
                      maxdt = maxtime;
                    }
                    if ( mintime < 0 or mintime > maxdt or maxtime < dt) {
                      ray_valid = false;
                      break;
                    } else {
                      if (mintime > dt) {
                        dt = mintime;
                      }
                      if (maxtime < maxdt) {
                        maxdt = maxtime;
                      }
                    }
                  }
                  
                  if (ray_valid and dt < timeleft) {
                    vec3 newpos = start + newdir * (dt + 0.001f);
                    Block* startblock = curblock->freechild->get_local(SAFEFLOOR3(newpos), 1);
                    // cout << " startblock " << startblock << ' ' << SAFEFLOOR3(newpos) << ' ' << newpos << ' ' << dt << endl;
                    cout << "Before " << start << ' ' << newpos << endl;
                    Block* result = startblock->raycast(&newpos, newdir, timeleft - dt);
                    double time = glm::dot(newpos - (start + newdir * (dt + 0.001f)), dir);
                    cout << "After " << newpos << endl;
                    // cout << " result " << result << endl;
                    if (result != nullptr and time < best_time) {
                      *pos = curblock->freechild->box.transform_out(newpos);
                      curblock = result;
                      best_time = time;
                    }
                  }
                }
              }
            }
          }
        }
      }
      parblock = parblock->parent;
    }
  }
  
  return curblock;
}

bool Block::collide(Hitbox newbox, Hitbox* boxhit, float deltatime, FreeBlock* ignore) {
  for (int x = -1; x < 2; x ++) {
    for (int y = -1; y < 2; y ++) {
      for (int z = -1; z < 2; z ++) {
        ivec3 off (x,y,z);
        Block* block = get_global(globalpos + off * scale, scale);
        if (collide(newbox, block, boxhit, deltatime, ignore)) {
          return true;
        }
      }
    }
  }
  return false;
}

bool Block::collide_free(Hitbox newbox, Block* block, Hitbox* boxhit, float deltatime, FreeBlock* ignore) {
  while (block != nullptr) {
    for (FreeBlock* free = block->freechild; free != nullptr; free = free->freechild) {
      if (free != ignore) {
        if (collide(newbox, free, boxhit, deltatime, ignore)) {
          return true;
        }
      }
    }
    block = block->parent;
  }
  return false;
}

bool Block::collide(Hitbox newbox, Block* block, Hitbox* boxhit, float deltatime, FreeBlock* ignore) {
  if (block == nullptr) {
    return false;
  }
  
  if (block->continues) {
    for (int i = 0; i < csize3; i ++) {
      if (collide(newbox, block->children[i], boxhit, deltatime, ignore)) {
        return true;
      }
    }
  } else if (block->pixel->value != 0) {
    if (block->hitbox().collide(newbox, deltatime)) {
      *boxhit = block->hitbox();
      return true;
    }
  }
  
  if (collide_free(newbox, block, boxhit, deltatime, ignore)) {
    return true;
  }
  return false;
}
  
vec3 Block::force(vec3 amount) {
  if (freecontainer != nullptr) {
    // freecontainer->move(amount);
    return vec3(0,0,0);
  }
  return amount;
}





FreeBlock::FreeBlock(Hitbox newbox): Block(), box(newbox) {
  
}

FreeBlock::FreeBlock(Block block, Hitbox newbox): Block(block), box(newbox) {
  
}

void FreeBlock::set_parent(Block* nparent, Container* nworld, ivec3 ppos, int nscale) {
  if (highparent == nullptr or nparent == nullptr or highparent->world != nparent->world) {
    if (highparent != nullptr) highparent->world->remove_freeblock(this);
    if (nparent != nullptr) nparent->world->add_freeblock(this);
  }
  highparent = nparent;
  Block::set_parent(nullptr, nworld, this, ppos, nscale);
}

void FreeBlock::timestep_freeblock(float deltatime, Block* block) {
  float coltime = deltatime;
  
  if (block->freebox().collide(box, deltatime)) {
    if (block->freechild != nullptr and block->freechild->box.collide(box, deltatime, &coltime)) {
      if (coltime < maxtime) {
        maxtime = coltime;
        maxbox = block->freechild->box;
      }
      if (coltime < block->freechild->maxtime) {
        block->freechild->maxtime = coltime;
        block->freechild->maxbox = box;
      }
    }
    
    if (block->continues) {
      for (int i = 0; i < csize3; i ++) {
        timestep_freeblock(deltatime, block->children[i]);
      }
    }
  }
}

void FreeBlock::timestep(float deltatime) {
  return;
  maxtime = deltatime;
  maxbox = Hitbox();
  
  // freeblock -> world collisisions
  float coltime = deltatime;
  FreeBlockIter freeiter (highparent, box);
  for (int i = 0; i < freeiter.num_bases; i ++) {
    for (Pixel* pix : freeiter.bases[i]->iter()) {
      if (pix->parbl->hitbox().collide(box, deltatime, &coltime) and coltime < maxtime) {
        maxtime = coltime;
      }
    }
  }
  
  // freeblock -> freeblock collisiions
  for (int x = -1; x < 2; x ++) {
    for (int y = -1; y < 2; y ++) {
      for (int z = -1; z < 2; z ++) {
        Block* block = highparent->get_global(highparent->globalpos + ivec3(x,y,z) * highparent->scale, highparent->scale);
        if (block != nullptr and world != block->world) {
          
        }
        // timestep_freeblock(deltatime, block);
      }
    }
  }
  
  box.timestep(maxtime);
}
  

void FreeBlock::move(vec3 amount, quat rot) {
  Hitbox newbox = box;
  newbox.velocity = amount;
  newbox.angular_vel = rot;
  Hitbox boxhit;
  if (highparent->collide(newbox, &boxhit, 1, this)) {
    float col_time = -1;
    if (newbox.collide(boxhit, 1, &col_time)) {
      cout << "collide " << ' ' << col_time << ' ' << boxhit << endl;
      newbox.position += amount * col_time;
      newbox.rotation += rot * col_time;
    } else {
      cout << "BRUD " << endl;
    }
  } else {
    newbox.position += amount;
    newbox.rotation += rot;
  }
  newbox.velocity = box.velocity;
  newbox.angular_vel = box.angular_vel;
  cout << " newbox " << newbox << endl;
  set_box(newbox);
}
  
bool FreeBlock::try_set_box(Hitbox newbox) {
  Hitbox boxhit;
  if (highparent->collide(newbox, &boxhit, 1, this)) {
    return false;
  }
  set_box(newbox);
  return true;
}

void FreeBlock::set_box(Hitbox newbox) {
  
  if (!highparent->freebox().contains(newbox)) {
    ivec3 dir (0,0,0);
    for (int x = -1; x < 2; x ++) {
      for (int y = -1; y < 2; y ++) {
        for (int z = -1; z < 2; z ++) {
          Hitbox freebox = highparent->freebox();
          freebox.position += vec3(x, y, z) * float(scale*2);
          if (freebox.contains(newbox)) {
            dir = ivec3(x, y, z);
            break;
          }
        }
      }
    }
    
    Block* newparent = get_global(highparent->globalpos + dir * scale * 2, scale*2);
    if (dir == ivec3(0,0,0) and newparent != nullptr) { // Todo what happens when freeblock leave loaded chunks
      while (newparent->scale > scale*2) {
        newparent->subdivide();
        newparent = newparent->get_global(highparent->globalpos + dir * scale * 2, scale*2);
      }
      
      highparent->remove_freechild(this);
      newparent->add_freechild(this);
    }
  }
  box = newbox;
  flags &= ~RENDER_FLAG;
  set_flag(RENDER_FLAG);
  for (Pixel* pix : iter()) {
    pix->parbl->set_flag(RENDER_FLAG);
  }
}



void FreeBlock::expand(ivec3 dir) {
  // cout << "expand " << this << ' ' << dir << ' ' << globalpos << ' ' << scale << endl;
  // //Block tmpblock = std::move(*this);
  // lock();
  //
  // Pixel* oldpix = pixel;
  // Block* oldchilds[csize];
  // for (int i = 0; i < csize3; i ++) {
  //   oldchilds[i] = children[i];
  // }
  // bool oldcont = continues;
  // cout << "old " << oldpix << oldcont << endl;
  //
  // cout << "oldtest " << endl;
  // cout << this << ' ' << globalpos << ' ' << parentpos << ' ' << parent << ' ' << freecontainer << ' ' << scale << endl;
  // for (Pixel* pix : iter()) {
  //   Block* block = pix->parbl;
  //   cout << block << ' ' << block->globalpos << ' ' << block->parentpos
  //   << ' ' << block->parent << ' ' << block->freecontainer << ' ' << block->scale << endl;
  // }
  //
  // continues = false;
  // pixel = nullptr;
  // ivec3 newpos = (-dir + 1) / 2;
  //
  // // Cambiando el posicion de bloque libre
  // offset -= box.transform_out_dir(newpos * scale);
  // int newscale = scale * csize;
  // Block::set_parent(nullptr, world, this, parentpos, newscale);
  // cout << parentpos << ' ' << scale << ' ' << endl;
  //
  // // update freeblock pointers
  // vec3 pos = box.transform_out(ivec3(0,0,0));
  // vec3 dx = box.transform_out_dir(ivec3(1,0,0));
  // vec3 dy = box.transform_out_dir(ivec3(0,1,0));
  // vec3 dz = box.transform_out_dir(ivec3(0,0,1));
  // FreeBlockIter myiter (world, pos, dx, dy, dz, vec3(scale, scale, scale));
  // for (Pixel* pix : myiter) {
  //   pix->parbl->set_flag(RENDER_FLAG | LIGHT_FLAG);
  //   // pix->parbl->freechild = this;
  // }
  //
  // cout << "bouta divide" << endl;
  // divide();
  // cout << "dividado " << endl;
  // cout << newpos << endl;
  // for (int i = 0; i < csize3; i ++) {
  //   cout << " ." << i << endl;
  //   if (posof(i) == newpos) {
  //     if (oldcont) {
  //       Block* block = get(newpos);
  //       for (int i = 0; i < csize3; i ++) {
  //         block->set_child(posof(i), oldchilds[i]);
  //       }
  //       cout << " test " << block << ' ' << block->globalpos << ' ' << block->parentpos
  //       << ' ' << block->parent << ' ' << block->freecontainer << ' ' << block->scale << endl;
  //       for (Pixel* pix : block->iter()) {
  //         Block* block = pix->parbl;
  //         cout << block << ' ' << block->globalpos << ' ' << block->parentpos
  //         << ' ' << block->parent << ' ' << block->freecontainer << ' ' << block->scale << endl;
  //       }
  //     } else {
  //       get(newpos)->set_pixel(oldpix);
  //     }
  //   } else {
  //     get(posof(i))->set_pixel(new Pixel(0));
  //   }
  //   Block* block = get(posof(i));
  //   cout << block << ' ' << block->globalpos << ' ' << block->parentpos
  //   << ' ' << block->parent << ' ' << block->freecontainer << ' ' << block->scale << endl;
  // }
  //
  // cout << "Test " << endl;
  // cout << this << ' ' << globalpos << ' ' << parentpos << ' ' << parent << ' ' << freecontainer << ' ' << scale << endl;
  // for (Pixel* pix : iter()) {
  //   Block* block = pix->parbl;
  //   cout << block << ' ' << block->globalpos << ' ' << block->parentpos
  //   << ' ' << block->parent << ' ' << block->freecontainer << ' ' << block->scale << endl;
  // }
  //
  // cout << "done " << endl;
  // unlock();
  // set_flag(RENDER_FLAG | LIGHT_FLAG);
}











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
  parbl->set_flag(RENDER_FLAG);
}

void Pixel::rotate_uv(float* uvs, int rot) {
  const float points[] {uvs[0],uvs[1], uvs[2],uvs[3], uvs[4],uvs[5], uvs[8],uvs[9]};
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
    
    quat rot = parbl->getrotation();
    renderdata.pos.loc.pos = parbl->fglobalpos() + rot * vec3(parbl->scale/2.0f, parbl->scale/2.0f, parbl->scale/2.0f);
    renderdata.pos.loc.rot = rot;
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
      // Block* block = parbl->get_global((vec3(gpos) + parbl->scale/2.0f) + vec3(dir) * (parbl->scale/2.0f), parbl->scale, dir);
      // Block* block = parbl->get_global(gpos + dir * parbl->scale, parbl->scale);
      renderdata.type.faces[i].tex = 0;
      if (parbl->is_air(dir, value)) {
        renderdata.type.faces[i].tex = mat[i];
        renderdata.type.faces[i].rot = dirs[i];
        renderdata.type.faces[i].blocklight = parbl->get_blocklight(dir);
        renderdata.type.faces[i].sunlight = parbl->get_sunlight(dir);
        exposed = true;
      }
    }
    
    if (exposed) {
      if (blockdata->transparent) {
        if (lastvecs == transvecs and !render_index.isnull()) {
          lastvecs->edit(render_index, renderdata);
        } else {
          render_index = transvecs->add(renderdata);
          lastvecs = transvecs;
        }
      } else {
        if (lastvecs == allvecs and !render_index.isnull()) {
          allvecs->edit(render_index, renderdata);
        } else {
          render_index = allvecs->add(renderdata);
          lastvecs = allvecs;
        }
      }
    }
  }
}

void Pixel::set(int val, int newdirection, int newjoints[6]) {
  WRITE_LOCK;
  cout << int(val) << endl;
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
  
  for (ivec3 dir : dir_array) {
    for (Pixel* pix : parbl->iter_touching_side(dir)) {
      pix->render_update();
    }
  }
  
  /*
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
  }*/
}
  
void Pixel::render_update() {
  parbl->set_flag(RENDER_FLAG | LIGHT_FLAG);
}

void Pixel::random_tick() {
  
}

void Pixel::tick() { // ERR: race condition, render and tick threads race, render renders, tick causes falling
  return;
  if (value == 0) {
    if (group != nullptr) {
      group->del(this);
    }
    group = nullptr;
  } else if (group == nullptr) {
    BlockGroup* newgroup = new BlockGroup(world);
    newgroup->spread(this, nullptr, ivec3(0,0,0), 1000);
  }
}

void Pixel::erase_render() {
  if (render_index.index != -1) {
    lastvecs->del(render_index);
    render_index = RenderIndex::npos;
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
            pix->parbl->set_flag(LIGHT_FLAG);
          }
        } else if (tilepos != SAFEDIV(pix->parbl->globalpos, World::chunksize) and dir.y == -1 and pix->sunlight == lightmax) {
          pix->parbl->set_flag(LIGHT_FLAG);
        }
        if (changed) pix->parbl->set_flag(RENDER_FLAG);
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
            pix->parbl->set_flag(LIGHT_FLAG);
          }
        }
        if (changed) pix->parbl->set_flag(RENDER_FLAG);
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
            pix->parbl->set_flag(LIGHT_FLAG);
          }
        } else if (SAFEMOD(pix->parbl->globalpos, World::chunksize) == tilepos
         and dir.y == -1 and pix->sunlight == lightmax) {
          pix->parbl->set_flag(LIGHT_FLAG);
        }
        if (changed) pix->parbl->set_flag(RENDER_FLAG);
      }
    }
  }
}

void Pixel::calculate_blocklight(unordered_set<ivec3,ivec3_hash>& next_poses) {
  if ((value != 0 and !blocks->blocks[value]->transparent)) {
    return;
  }
  const int decrement = 1;
  int oldblocklight = blocklight;BlockContainer::BlockContainer(Block* b): block(b), allocated(false) {
	block->set_parent(nullptr, this, nullptr, ivec3(0,0,0), b->scale);
}

BlockContainer::BlockContainer(int scale): allocated(true) {
  block = new Block();
	block->set_parent(nullptr, this, nullptr, ivec3(0,0,0), scale);
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

void BlockContainer::set_global(ivec3 pos, int w, Blocktype val, int direction, int newjoints[6]) {
  block->set_global(pos, w, val, direction, newjoints);
}

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
            pix->parbl->set_flag(LIGHT_FLAG);
          }
        }
        if (changed) pix->parbl->set_flag(RENDER_FLAG);
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


#endif
