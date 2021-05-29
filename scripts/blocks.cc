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

Block::Block(Block* childs, uint8 nwinding): continues(true), children(childs), winding(nwinding) {
  
}

Block::~Block() {
  if (continues) {
    delete[] children;
  } else if (pixel != nullptr) {
    delete pixel;
  }
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
    for (int i = 0; i < csize3; i ++) {
      children[i].set_parent(this, world, freecontainer, posof(i), scale / csize);
    }
  }
  set_render_flag();
  set_light_flag();
}

void Block::set_child(ivec3 pos, Block* block) { ASSERT(ISCHUNK)
  *get(pos) = std::move(*block);
  get(pos)->set_parent(this, world, freecontainer, pos, scale / csize);
}

void Block::set_pixel(Pixel* pix) { ASSERT(ISUNDEF||ISPIXEL)
  pix->set_block(this);
  if (pixel != nullptr) {
    delete pixel;
  }
  pixel = pix;
}

void Block::set_freeblock(FreeBlock* nfreeblock) {
  freeblock = nfreeblock;
  freeblock->set_parent(this);
  set_render_flag();
  set_light_flag();
}

bool Block::expand_freeblock(FreeBlock* nfreeblock) {
  freeblock = nfreeblock;
  return true;
}



Block* Block::get(ivec3 pos) {
  return children + (pos.x*csize*csize + pos.y * csize + pos.z);
}

Block* Block::get(int x, int y, int z) {
  return children + (x*csize*csize + y*csize + z);
}

const Block* Block::get(ivec3 pos) const {
  return children + (pos.x*csize*csize + pos.y * csize + pos.z);
}

const Block* Block::get(int x, int y, int z) const {
  return children + (x*csize*csize + y*csize + z);
}

int Block::indexof(ivec3 pos) const {
  if (winding == 0) {
    return pos.x*4 + pos.y*2 + pos.z;
  } else if (winding & 0b10000000) {
    return (pos.x)*(winding&0b100000) + (pos.x*4)*(winding&0b100)
    + (pos.y)*(winding&0b10000) + (pos.y*4)*(winding&0b10)
    + (pos.z)*(winding&0b1000) + (pos.z*4)*(winding&0b1);
  } else if (winding & 0b01000000) {
    return pos.x*(winding&0b100 != 0) + pos.y*(winding&0b10 != 0) + pos.z*(winding&0b1 != 0);
  }
  ERR("invalid winding");
  return 0;
}

int Block::indexof(int x, int y, int z) const {
  if (winding == 0) {
    return x*4 + y*2 + z;
  } else if (winding & 0b10000000) {
    return x*(winding&0b100000) + (x*4)*(winding&0b100)
    + y*(winding&0b10000) + (y*4)*(winding&0b10)
    + z*(winding&0b1000) + (z*4)*(winding&0b1);
  } else if (winding & 0b01000000) {
    return x*(winding&0b100 != 0) + y*(winding&0b10 != 0) + z*(winding&0b1 != 0);
  }
  ERR("invalid winding");
  return 0;
}

ivec3 Block::posof(int index) const {
  if (winding == 0) {
    return ivec3(index/(csize*csize), index/csize%csize, index%csize);
  } else if (winding & 0b10000000) {
    return ivec3(
      (index%4)*(winding&0b100000) + (index/4)*(winding&0b100),
      (index%4)*(winding&0b10000) + (index/4)*(winding&0b10),
      (index%4)*(winding&0b1000) + (index/4)*(winding&0b1)
    );
  } else if (winding & 0b01000000) {
    return ivec3(index*(winding&0b100 != 0), index*(winding&0b10 != 0), index*(winding&0b1 != 0));
  }
  ERR("invalid winding");
  return ivec3(0,0,0);
}


quat Block::getrotation() const {
  quat rot(1,0,0,0);
  // cout << rot.w << endl;
  // cout << ((float*)&rot)[0] << ((float*)&rot)[1] << ((float*)&rot)[2] << ((float*)&rot)[3] << endl;
  FreeBlock* free = freecontainer;
  while (free != nullptr) {
    rot = rot * free->rotation;
    free = free->highparent->freecontainer;
  }
  return rot;
}

vec3 Block::fglobalpos() const {
  vec3 pos = globalpos;
  FreeBlock* free = freecontainer;
  while (free != nullptr) {
    pos = free->transform_out(pos);
    free = free->highparent->freecontainer;
  }
  return pos;
}

Block* Block::get_global(int x, int y, int z, int w) {
  return get_global(ivec3(x,y,z), w);
}
  
Block* Block::get_global(ivec3 pos, int w) {
  Block* curblock = this;
  while (SAFEDIV(pos, curblock->scale) != SAFEDIV(curblock->globalpos, curblock->scale)) {
    if (curblock->parent == nullptr) {
      if (curblock->freecontainer != nullptr) {
        pos = curblock->freecontainer->transformi_out(pos);
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
        pos = curblock->freecontainer->transform_out(pos);
        dir = curblock->freecontainer->transform_out_dir(dir);
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
  
  if (curblock->freeblock != nullptr) {
    FreeBlock* freeblock = curblock->freeblock;
    while (freeblock != nullptr) {
      Block* block = freeblock->get_local(freeblock->transform_into(pos), w, freeblock->transform_into_dir(dir));
      if (block != nullptr) {
        return block;
      }
    }
  }
  
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

void Block::divide(uint8 nwinding) { ASSERT(!continues)
  if (pixel != nullptr) {
    delete pixel;
  }
  winding = nwinding;
  children = new Block[csize3];
  continues = true;
  for (int i = 0; i < csize3; i ++) {
    children[i].set_parent(this, world, freecontainer, posof(i), scale / csize);
  }
}

void Block::subdivide() { ASSERT(!continues)
  Pixel pix = *pixel;
  divide();
  for (int i = 0; i < csize3; i ++) {
    children[i].set_pixel(new Pixel(pix));
  }
}

void Block::join() { ASSERT(ISCHUNK)
  delete[] children;
  continues = false;
  pixel = nullptr;
}

void Block::changewinding(uint8 nwinding) {
  Block blocks[csize3];
  for (int i = 0; i < csize3; i ++) {
    blocks[i] = *get(posof(i));
  }
  winding = nwinding;
  for (int i = 0; i < csize3; i ++) {
    *get(posof(i)) = blocks[i];
  }
}


void Block::to_file(ostream& ofile) const {
  if (continues) {
    ofile << char(0b11000000);
    for (int i = 0; i < csize3; i ++) {
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
    for (int i = 0; i < csize3; i ++) {
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
    for (int i = 0; i < csize3; i ++) {
      children[i].set_all_render_flags();
    }
  } else if (freeblock != nullptr) {
    freeblock->set_all_render_flags();
  }
}

void Block::set_light_flag() {
  light_flag = true;
  Block* curblock = parent;
  while (curblock != nullptr and !curblock->light_flag) {
    curblock->light_flag = true;
    if (curblock->parent == nullptr and curblock->freecontainer != nullptr) {
      curblock = curblock->freecontainer;
    } else {
      curblock = curblock->parent;
    }
  }
}

void Block::render(RenderVecs* vecs, RenderVecs* transvecs, uint8 faces, bool render_null) {
  if (render_flag) {
    render_flag = false;
    if (continues) {
      for (int i = 0; i < csize3; i ++) {
        children[i].render(vecs, transvecs, faces, render_null);
      }
    } else {
      pixel->render(vecs, transvecs, faces, render_null);
      if (freeblock != nullptr) {
        freeblock->render(vecs, transvecs, faces, render_null);
      }
    }
  }
}

void Block::lighting_update()  {
  if (light_flag) {
    light_flag = false;
    if (continues) {
      for (int i = 0; i < csize3; i ++) {
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
  cout << "Raycast " << *pos << ' ' << dir << ' ' << timeleft << endl;
  
  while (curblock != nullptr and (curblock->pixel->value == 0 or curblock->pixel->value == 7)) {
    cout << " raycast " << curblock << ' ' << *pos << endl;
    if (curblock->freeblock != nullptr) {
      vec3 start = curblock->freeblock->transform_into(*pos);
      vec3 newdir = curblock->freeblock->transform_into_dir(dir);
      cout << start << ' ' << newdir << endl;
  		float dt = 0;
  		float maxdt = -1;
  		bool ray_valid = true;
  		for (int axis = 0; axis < 3 and ray_valid; axis ++) {
  			float mintime;
  			float maxtime;
  			
  			if (start[axis] < 0) {
  				mintime = (-start[axis]) / newdir[axis];
  				maxtime = (curblock->freeblock->scale - start[axis]) / newdir[axis];
  			} else if (start[axis] > curblock->freeblock->scale) {
  				mintime = (curblock->freeblock->scale - start[axis]) / newdir[axis];
  				maxtime = (-start[axis]) / newdir[axis];
  			} else {
  				mintime = 0;
  				if (newdir[axis] < 0) {
  					maxtime = (-start[axis]) / newdir[axis];
  				} else {
  					maxtime = (curblock->freeblock->scale - start[axis]) / newdir[axis];
  				}
  			}
        cout << mintime << ' ' << maxtime << endl;
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
        cout << " startblock " << startblock << ' ' << SAFEFLOOR3(newpos) << ' ' << newpos << ' ' << dt << endl;
        Block* result = startblock->raycast(&newpos, newdir, timeleft - dt);
        cout << " result " << result << endl;
        if (result != nullptr) {
          return result;
        }
      }
    }
      
    
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

  




FreeBlock::FreeBlock(vec3 newoffset, quat newrotation): Block(), offset(newoffset), rotation(newrotation) {
  
}

void FreeBlock::set_parent(Block* nparent, Container* nworld, ivec3 ppos, int nscale) {
  highparent = nparent;
  Block::set_parent(nullptr, nworld, this, ppos, nscale);
}

void FreeBlock::set_parent(Block* nparent) {
  highparent = nparent;
}

void FreeBlock::expand(ivec3 dir) {
  cout << "expand " << this << ' ' << dir << endl;
  Block tmpblock = std::move(*this);
  continues = false;
  pixel = nullptr;
  ivec3 newpos = (-dir + 1) / 2;
  
  // Cambiando el posicion de bloque libre
  offset -= transform_out_dir(newpos * scale);
  int newscale = scale * csize;
  Block::set_parent(nullptr, world, this, parentpos, newscale);
  cout << parentpos << ' ' << scale << ' ' << endl;
  
  cout << "bouta divide" << endl;
  divide();
  cout << "dividado " << endl;
  cout << newpos << endl;
  for (int i = 0; i < csize3; i ++) {
    cout << " ." << i << endl;
    if (posof(i) == newpos) {
      set_child(newpos, &tmpblock);
      get(newpos)->set_pixel(get(newpos)->pixel);
    } else {
      get(posof(i))->set_pixel(new Pixel(0));
    }
  }
  
  cout << "Test " << endl;
  cout << this << ' ' << globalpos << ' ' << parentpos << ' ' << parent << ' ' << freecontainer << ' ' << scale << endl;
  for (Pixel* pix : iter()) {
    Block* block = pix->parbl;
    cout << block << ' ' << block->globalpos << ' ' << block->parentpos
    << ' ' << block->parent << ' ' << block->freecontainer << ' ' << block->scale << endl;
  }
  
  cout << "done " << endl;
  
}


vec3 FreeBlock::transform_into(vec3 pos) const {
  return glm::inverse(rotation) * (pos - offset);
}

ivec3 FreeBlock::transformi_into(ivec3 pos) const {
  vec3 result = glm::inverse(rotation) * (vec3(pos) - offset);
  return SAFEFLOOR3(result);
}


vec3 FreeBlock::transform_into_dir(vec3 dir) const {
  return glm::inverse(rotation) * dir;
}

ivec3 FreeBlock::transformi_into_dir(ivec3 dir) const {
  vec3 result = glm::inverse(rotation) * vec3(dir);
  return SAFEFLOOR3(result);
}


vec3 FreeBlock::transform_out(vec3 pos) const {
  return offset + rotation * pos;
}

ivec3 FreeBlock::transformi_out(ivec3 pos) const {
  vec3 result = offset + rotation * vec3(pos);
  return SAFEFLOOR3(result);
}


vec3 FreeBlock::transform_out_dir(vec3 dir) const {
  return rotation * dir;
}

ivec3 FreeBlock::transformi_out_dir(ivec3 dir) const {
  vec3 result = rotation * vec3(dir);
  return SAFEFLOOR3(result);
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
    
    renderdata.pos.loc.pos = parbl->fglobalpos();
    renderdata.pos.loc.rot = parbl->getrotation();
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
  // if (group != nullptr and !group->consts[4]) {
    // block_entities.push_back(new FallingBlockEntity(world, group, this));
  // }
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











FreeBlockIter::FreeBlockIter(Collider* world, vec3 pos, vec3 dx, vec3 dy, vec3 norm, float scale):
position(pos), dirx(dx), diry(dy), normal(norm), facescale(scale) {
  // cout << "new freiter " << pos << ' ' << dx << ' ' << dy << ' ' << norm << ' ' << scale << endl;
  increment_func = [] (ivec3 pos, ivec3 startpos, ivec3 endpos) {
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
  
  Block* pointblocks[4] {
    world->get_global(ivec3(pos), 1),
    world->get_global(ivec3(pos+dx), 1),
    world->get_global(ivec3(pos+dy+dx), 1),
    world->get_global(ivec3(pos+dy), 1)
  };
  
  int minscale = World::chunksize;
  for (int i = 0; i < 4; i ++) {
    if (pointblocks[i]->scale < minscale) {
      minscale = pointblocks[i]->scale;
    }
  }
  
  while (minscale < World::chunksize and (pointblocks[0] != pointblocks[1]
  or pointblocks[1] != pointblocks[2] or pointblocks[2] != pointblocks[3])) {
    for (int i = 0; i < 4; i ++) {
      if (pointblocks[i]->scale == minscale) {
        pointblocks[i] = pointblocks[i]->parent;
      }
    }
    minscale *= csize;
  }
  
  for (int i = 0; i < 4; i ++) {
    if (i == 0 or bases.back() != pointblocks[i]) {
      bases.push_back(pointblocks[i]);
    }
  }
}


Pixel* FreeBlockIter::iterator::operator*() {
  // cout << "-- Used " << pix->parbl->globalpos << ' ' << pix->parbl->scale << endl;
  return pix;
}

bool FreeBlockIter::iterator::in_cube(vec3 pos, vec3 norm, vec3 center, float scale) {
  pos += norm * 0.001f;
  return  center.x - scale <= pos.x and pos.x <= center.x + scale
      and center.y - scale <= pos.y and pos.y <= center.y + scale
      and center.z - scale <= pos.z and pos.z <= center.z + scale;
}

bool FreeBlockIter::iterator::in_face(Block* block) {
  float scale = block->scale / 2.0f;
  vec3 center = vec3(block->globalpos) + scale;
  vec2 planepoint (
    glm::dot(center - parent->position, parent->dirx),
    glm::dot(center - parent->position, parent->diry)
  );
  // cout << " before inface " << planepoint.x << ' ' << planepoint.y << ' ' << parent->position << ' ' << parent->dirx << ' ' << parent->diry << endl;
  planepoint = glm::max(glm::min(planepoint, vec2(parent->facescale, parent->facescale)), vec2(0,0));
  vec3 closepoint = planepoint.x * parent->dirx + planepoint.y * parent->diry + parent->position;
  // cout << " inface  " << closepoint << ' ' << planepoint.x << ',' << planepoint.y << ' ' << center << ' ' << scale << endl;
  return in_cube(closepoint, parent->normal, center, scale);
}

FreeBlockIter::iterator FreeBlockIter::iterator::operator++() {
  Block* block = pix->parbl;
  pix = nullptr;
  increment(block);
  return *this;
}
  
void FreeBlockIter::iterator::increment(Block* block) {
  while (block->parent != base->parent and block->parentpos == parent->end_pos) {
    block = block->parent;
  }
  if (block->parent == base->parent) {
    baseindex ++;
    if (baseindex >= parent->bases.size()) {
      pix = (Pixel*)0x1; // horrible, lo cambiare pronto
      cout << " Termino " << endl;
      return;
    } else {
      base = parent->bases[baseindex];
      get_to_pix(base);
      return;
    }
  }
  ivec3 pos = parent->increment_func(block->parentpos, parent->start_pos, parent->end_pos);
  block = block->parent->get(pos);
  get_to_pix(block);
}

void FreeBlockIter::iterator::get_to_pix(Block* block) {
  // cout << "get_to_pix ( " << block << ' ' << block->globalpos << ' ' << block->scale << endl;
  while (pix == nullptr) {
    if (!in_face(block)) {
      // cout << " not in face " << endl;
      increment(block);
    } else if (block->continues) {
      // cout << " inner " << endl;
      block = block->get(parent->start_pos);
    } else {
      // cout << " setting " << endl;
      pix = block->pixel;
    }
  }
}

FreeBlockIter::iterator FreeBlockIter::begin() {
  // if (base == nullptr) {
  //   return end();
  // }
  iterator iter {this, bases[0], 0, nullptr};
  iter.get_to_pix(bases[0]);
  return iter;
}

FreeBlockIter::iterator FreeBlockIter::end() {
  return {this, nullptr, 0, (Pixel*)0x1};
}

bool operator!=(const FreeBlockIter::iterator& iter1, const FreeBlockIter::iterator& iter2) {
  return iter1.parent != iter2.parent or iter1.pix != iter2.pix;
}








Pixel* BlockTouchSideIter::iterator::operator*() {
  if (parent->free) {
    return *freeiter;
  } else {
    return *blockiter;
  }
}

BlockTouchSideIter::iterator BlockTouchSideIter::iterator::operator++() {
  if (parent->free) {
    ++freeiter;
  } else {
    ++blockiter;
  }
  return *this;
}

BlockTouchSideIter::BlockTouchSideIter(Block* block, ivec3 dir) {
  Block* side = block->get_local(block->globalpos + dir * block->scale, block->scale);
  if (side == nullptr) {
    if (block->freecontainer != nullptr) {
      // cout << "Free iter " << block << ' ' << dir << endl;
      vec3 norm = dir;
      vec3 dx;
      vec3 dy;
      vec3 position = block->globalpos;
      if (dir_to_index(dir) < 3) {
        position += block->scale;
        dx = dir_array[(dir_to_index(dir)+1) % 3 + 3];
        dy = dir_array[(dir_to_index(dir)+2) % 3 + 3];
      } else {
        dx = dir_array[(dir_to_index(dir)+1) % 3];
        dy = dir_array[(dir_to_index(dir)+2) % 3];
      }
      
      free = true;
      freeiter = FreeBlockIter(block->world,
        block->freecontainer->transform_out(position),
        block->freecontainer->transform_out_dir(dx),
        block->freecontainer->transform_out_dir(dy),
        block->freecontainer->transform_out_dir(norm),
        block->scale
      );
    } else {
      free = false;
      blockiter = BlockIter{.base = nullptr};
    }
  } else {
    free = false;
    blockiter = side->iter_side(-dir);
  }
}

BlockTouchSideIter::iterator BlockTouchSideIter::begin() {
  iterator iter;
  iter.parent = this;
  if (free) {
    iter.freeiter = freeiter.begin();
  } else {
    iter.blockiter = blockiter.begin();
  }
  return iter;
}

BlockTouchSideIter::iterator BlockTouchSideIter::end() {
  iterator iter;
  iter.parent = this;
  if (free) {
    iter.freeiter = freeiter.end();
  } else {
    iter.blockiter = blockiter.end();
  }
  return iter;
}

bool operator!=(const BlockTouchSideIter::iterator& iter1, const BlockTouchSideIter::iterator& iter2) {
  if (iter1.parent != iter2.parent) {
    return false;
  }
  if (iter1.parent->free) {
    return iter1.freeiter != iter2.freeiter;
  } else {
    return iter1.blockiter != iter2.blockiter;
  }
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
  // cout << "start p " << block->globalpos << ' ' << block->parentpos << endl;
  return {this, block->pixel};
}

ConstBlockIter::iterator ConstBlockIter::end() {
  return {this, nullptr};
}

bool operator!=(const ConstBlockIter::iterator& iter1, const ConstBlockIter::iterator& iter2) {
  return iter1.parent != iter2.parent or iter1.pix != iter2.pix;
}

#endif
