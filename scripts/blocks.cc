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
    delete pixel;
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
  set_render_flag();
  set_light_flag();
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
    delete pixel;
  }
  pixel = pix;
  set_render_flag();
  set_light_flag();
}

Pixel* Block::swap_pixel(Pixel* pix) { ASSERT(ISPIXEL)
  pix->set_block(this);
  Pixel* old = pixel;
  pixel = pix;
  set_render_flag();
  set_light_flag();
  return old;
}

void Block::set_freeblock(FreeBlock* nfreeblock) {
  int i = 0;
  // while (i < 8 and freeblocks[i] == nullptr) i ++;
  // freeblocks[i] = nfreeblock;
  // freeblocks[i]->set_parent(this);
  set_render_flag();
  set_light_flag();
}

// bool Block::expand_freeblock(FreeBlock* nfreeblock) {
//   if (pixel->value == 0 or pixel->value == 7) {
//     freeblock = nfreeblock;
//     return true;
//   }
//   return false;
// }



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
  quat rot(1,0,0,0);
  // cout << rot.w << endl;
  // cout << ((float*)&rot)[0] << ((float*)&rot)[1] << ((float*)&rot)[2] << ((float*)&rot)[3] << endl;
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

Hitbox Block::hitbox() {
  if (freecontainer == nullptr) {
    return local_hitbox();
  } else {
    return freecontainer->box.transform_out(local_hitbox());
  }
}

Block* Block::get_global(int x, int y, int z, int w) {
  return get_global(ivec3(x,y,z), w);
}
  
Block* Block::get_global(ivec3 pos, int w) {
  Block* curblock = this;
  while (SAFEDIV(pos, curblock->scale) != SAFEDIV(curblock->globalpos, curblock->scale)) {
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
  
  // for (int i = 0; i < csize3 and curblock->freeblocks[i] != nullptr; i ++) {
  //   FreeBlock* freeblock = curblock->freeblocks[i]->freecontainer;
  //   Block* block = freeblock->get_local(freeblock->box.transform_into(pos), w, freeblock->box.transform_into_dir(dir));
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

void Block::set_global(ivec3 pos, int w, int val, int direc, int joints[6]) {
  cout << " setting globla " << pos << ' ' << w << endl;
  Block* curblock = this;
  while (SAFEDIV(pos, curblock->scale) != SAFEDIV(curblock->globalpos, curblock->scale)) {
    if (curblock->parent == nullptr) {
      if (curblock->freecontainer != nullptr) {
        cout << " antes posicion " << pos << endl;
        ivec3 expand_dir = glm::sign(SAFEDIV(pos, curblock->scale) - SAFEDIV(curblock->globalpos, curblock->scale));
        ivec3 expand_off = (-expand_dir + 1) / 2;
        pos += expand_off * curblock->scale;
        cout << " despues posicion " << pos << ' ' << curblock->scale << endl;
        curblock->freecontainer->expand(expand_dir);
        cout << curblock->scale << endl;
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
    delete pixel;
  }
  continues = true;
  for (int i = 0; i < csize3; i ++) {
    children[i] = nullptr;
  }
}

void Block::subdivide() { ASSERT(!continues)
  Pixel pix = *pixel;
  divide();
  for (int i = 0; i < csize3; i ++) {
    set_child(posof(i), new Block(new Pixel(pix)));
  }
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

void Block::set_render_flag() {
  if (locked) return;
  render_flag = true;
  Block* curblock = parent;
  while (curblock != nullptr and !curblock->render_flag and !curblock->locked) {
    curblock->render_flag = true;
    if (curblock->parent == nullptr and curblock->freecontainer != nullptr) {
      curblock = curblock->freecontainer;
    } else {
      curblock = curblock->parent;
    }
  }
}

void Block::set_all_render_flags() {
  render_flag = true;
  if (continues) {
    for (CHILDREN_LOOP(i)) {
      children[i]->set_all_render_flags();
    }
  }
  // for (FREECHILDREN_LOOP(i)) {
  //   freeblocks[i]->freecontainer->set_all_render_flags();
  // }
}

void Block::set_light_flag() {
  if (locked) return;
  light_flag = true;
  Block* curblock = parent;
  while (curblock != nullptr and !curblock->light_flag and !curblock->locked) {
    curblock->light_flag = true;
    if (curblock->parent == nullptr and curblock->freecontainer != nullptr) {
      curblock = curblock->freecontainer;
    } else {
      curblock = curblock->parent;
    }
  }
}

void Block::render(RenderVecs* vecs, RenderVecs* transvecs, uint8 faces, bool render_null) {
  if (render_flag and !locked) {
    render_flag = false;
    if (continues) {
      for (CHILDREN_LOOP(i)) {
        children[i]->render(vecs, transvecs, faces, render_null);
      }
    } else {
      pixel->render(vecs, transvecs, faces, render_null);
      // for (FREECHILDREN_LOOP(i)) {
      //   freeblocks[i]->render(vecs, transvecs, faces, render_null);
      // }
    }
  }
}

void Block::lighting_update()  {
  if (light_flag and !locked) {
    light_flag = false;
    if (continues) {
      for (CHILDREN_LOOP(i)) {
        children[i]->lighting_update();
      }
    } else {
      pixel->lighting_update();
      // for (FREECHILDREN_LOOP(i)) {
      //   freeblocks[i]->lighting_update();
      // }
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
  for (const Pixel* pix : iter_touching_side(dir)) {
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
  
  while (curblock != nullptr and (curblock->pixel->value == 0 or curblock->pixel->value == 7)) {
    // cout << " raycast " << curblock << ' ' << *pos << endl;
    /*
    for (int i = 0; i < csize3 and curblock->freeblocks[i] != nullptr; i ++) {
      FreeBlock* freeblock = curblock->freeblocks[i]->freecontainer;
      // cout << " going into free block" << endl;
      vec3 start = freeblock->box.transform_into(*pos);
      vec3 newdir = freeblock->box.transform_into_dir(dir);
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
        Block* startblock = curblock->freeblock->get_local(SAFEFLOOR3(newpos), 1);
        // cout << " startblock " << startblock << ' ' << SAFEFLOOR3(newpos) << ' ' << newpos << ' ' << dt << endl;
        cout << "Before " << start << ' ' << newpos << endl;
        Block* result = startblock->raycast(&newpos, newdir, timeleft - dt);
        cout << "After " << newpos << endl;
        // cout << " result " << result << endl;
        if (result != nullptr) {
          *pos = curblock->freeblock->box.transform_out(newpos);
          return result;
        }
      }
    }*/
      
    
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
      return nullptr;
    }
    curblock = curblock->get_local(SAFEFLOOR3(*pos), 1);
  }
  
  return curblock;
}

  




FreeBlock::FreeBlock(Hitbox newbox): Block(), box(newbox) {
  
}

void FreeBlock::set_parent(Block* nparent, Container* nworld, ivec3 ppos, int nscale) {
  highparent = nparent;
  Block::set_parent(nullptr, nworld, this, ppos, nscale);
}

void FreeBlock::set_parent(Block* nparent) {
  highparent = nparent;
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
  //   pix->parbl->set_render_flag();
  //   pix->parbl->set_light_flag();
  //   // pix->parbl->freeblock = this;
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
  // set_render_flag();
  // set_light_flag();
}


void FreeBlock::set_box(Hitbox newbox) {
  box = newbox;
  for (Pixel* pix : iter()) {
    pix->parbl->set_render_flag();
  }
}

bool FreeBlock::try_set_box(Hitbox box) {
  // vec3 dx = box.transform_out_dir(vec3(1,0,0));
  // vec3 dy = box.transform_out_dir(vec3(0,1,0));
  // vec3 dz = box.transform_out_dir(vec3(0,0,1));
  // for (Pixel* pix : iter()) {
  //   if (pix->value != 0) {
  //     vec3 pos = box.transform_out(pix->parbl->globalpos);
  //
  //   }
  // }
  // return true;
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
      Block* block = parbl->get_global(gpos + dir * parbl->scale, parbl->scale);
      renderdata.type.faces[i].tex = 0;
      if (block != nullptr and parbl->is_air(dir, value)) {
        renderdata.type.faces[i].tex = mat[i];
        renderdata.type.faces[i].rot = dirs[i];
        renderdata.type.faces[i].blocklight = parbl->get_blocklight(dir);
        renderdata.type.faces[i].sunlight = parbl->get_sunlight(dir);
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
    erase_render();
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
    
    erase_render();
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
  parbl->set_render_flag();
  parbl->set_light_flag();
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

void BlockContainer::set_global(ivec3 pos, int w, int val, int direction, int newjoints[6]) {
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


#endif
