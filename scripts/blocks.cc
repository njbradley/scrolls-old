#ifndef BLOCKS
#define BLOCKS

#include "blocks.h"

#include <cmath>
#include "cross-platform.h"
#include "rendervec.h"
#include "blockdata.h"
#include "tiles.h"
#include "blockphysics.h"
#include "world.h"
#include "entity.h"
#include "mobs.h"
#include "game.h"

#include "generative.h"


const float lightmax = 20.0f;

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

Block::~Block() {
  
}

void Block::set_render_flag() {
  render_flag = true;
  if (parent != nullptr and !parent->render_flag) {
    parent->set_render_flag();
  }
}

void Block::set_light_flag() {
  light_flag = true;
  if (parent != nullptr and !parent->light_flag) {
    parent->set_light_flag();
  }
}

void Block::world_to_local(int x, int y, int z, int* lx, int* ly, int* lz) const {
    *lx = x-(x/scale*scale);
    *ly = y-(y/scale*scale);
    *lz = z-(z/scale*scale);
    //cout << scale << " -----scale ------" << endl;
}

void Block::global_position(int* gx, int* gy, int* gz) const {
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

vec3 Block::get_position() const {
  int x,y,z;
  global_position(&x,&y,&z);
  return vec3(x,y,z);
}

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

ConstBlockIter Block::const_iter_side(ivec3 dir) const {
  ivec3 startpos (0,0,0);
  ivec3 endpos (csize-1, csize-1, csize-1);
  startpos += (dir+1)/2 * (csize-1);
  endpos -= (1-dir)/2 * (csize-1);
  return const_iter(startpos, endpos);
}

Block* Block::from_file(istream& ifile, int px, int py, int pz, int scale, Chunk* parent, Tile* tile) {
    if (ifile.eof()) {
      cout << "ERR: Unexpected eof when reading block data file" << endl;
      return nullptr;
    }
    char c;
    c = ifile.peek();
    std::this_thread::yield();
    if (c == char(0b11000000)) {
        ifile.get();
        Chunk* chunk = new Chunk(px, py, pz, scale, parent);
        for (int x = 0; x < csize; x ++) {
            for (int y = 0; y < csize; y ++) {
                for (int z = 0; z < csize; z ++) {
                    chunk->blocks[x][y][z] = Block::from_file(ifile, x, y, z, scale/csize, chunk, tile);
                }
            }
        }
        // ifile.read(&c,1);
        // if (c != 0b) {
        //     cout << "error, mismatched {} in save file" << endl;
        // }
        return (Block*) chunk;
    } else {
        bool value_set = false;
        char type;
        unsigned int data;
        int direction = -1;
        while (!value_set and !ifile.eof()) {
          read_pix_val(ifile, &type, &data);
          if (type == 0b00) {
            c = data;
            value_set = true;
          } else if (type == 0b01) {
            direction = data;
          } else {
            cout << "ERR: unknown type tag in block data file '" << (type & 0b10) << (type & 0b1) << "'" << endl;
          }
        }
        Pixel* p = new Pixel(px, py, pz, c, scale, parent, tile);
        if (direction != -1) {
          p->direction = direction;
        }
        return (Block*) p;
    }
}

int Block::get_sunlight(int dx, int dy, int dz) const {
  int lightlevel = 0;
  int num = 0;
  
  for (const Pixel* pix : const_iter_side(ivec3(dx, dy, dz))) {
    if (pix->is_air(dx, dy, dz)) {
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
    if (pix->is_air(dx, dy, dz)) {
      int light;
      const ivec3 dirs[] = {{-1,0,0}, {0,-1,0}, {0,0,-1}, {1,0,0}, {0,1,0}, {0,0,1}};
      ivec3 source_dir = -dirs[pix->lightsource];
      if (source_dir.x == dx and source_dir.y == dy and source_dir.z == dz) {
        light = pix->blocklight;
      } else {
        light = pix->blocklight*0.875;
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
  
  

///////////////////////////////// CLASS PIXEL /////////////////////////////////////

Pixel::Pixel(int x, int y, int z, char new_val, int nscale, Chunk* nparent, Tile* ntile):
  Block(x, y, z, nscale, nparent), render_index(-1,0), value(new_val), tile(ntile), direction(0) {
    reset_lightlevel();
    if (value != 0) {
      direction = blocks->blocks[value]->default_direction;
    }
}

bool Pixel::continues() const {
    return false;
}

char Pixel::get() const {
    //cout << "pixel get px" << px << " py" << py << " pz" << pz << " s" << scale << " value" << value  << endl;
    return value;
}

Pixel* Pixel::get_pix() {
    return this;
}

Chunk* Pixel::get_chunk() {
    return nullptr;
}

const Pixel* Pixel::get_pix() const {
    return this;
}

const Chunk* Pixel::get_chunk() const {
    return nullptr;
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
    
    direction = newdirection;
    
    
    // if (physicsgroup != nullptr) {
    //   physicsgroup->update_flag = true;//block_poses.erase(ivec3(gx,gy,gz));
    //   //physicsgroup = nullptr;
    // }
    cout << "set group " << group << ' ' << gx << ' ' << gy << ' ' << gz << ' ' << scale << endl;
    if (group != nullptr and value == 0) {
      group->del(ivec3(gx, gy, gz));
    }
    if (tile != nullptr) {
      tile->world->block_update(gx, gy, gz);
    }
    //physicsgroup = nullptr;
    if (update) {
      //render_flag = true;
      //cout << gx << ' ' << gy << ' ' << gz << "-----------------------------" << render_index.first << ' ' << render_index.second << endl;
      //cout << "global position " << gx << ' ' << gy << ' ' << gz << endl;
      //light_flag = true;
      reset_lightlevel();
      render_update();
      Block* block;
      const ivec3 dir_array[6] =    {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
      for (ivec3 dir : dir_array) {
        dir *= scale;
        block = tile->world->get_global(gx+dir.x, gy+dir.y, gz+dir.z, scale);
        if (block != nullptr) {
          for (Pixel* pix : block->iter_side(-dir)) {
            pix->render_update();
          }
          int bx, by, bz;
          block->global_position(&bx, &by, &bz);
          //tile->world->block_update(bx, by, bz);
        }
      }
    }
}

void Pixel::random_tick() {
  return;
  int gx, gy, gz;
  global_position(&gx, &gy, &gz);
  if (value == blocks->names["dirt"] and tile->world->get(gx, gy+scale, gz) == 0) {
    set(blocks->names["grass"]);
    cout << "grass grown at " << gx << ' ' << gy << ' ' << gz << endl;
  }
  
  Block* above = tile->world->get_global(gx, gy+scale, gz, 1);
  if (value == blocks->names["grass"] and tile->world->get(gx, gy+scale, gz) == 0 and tile->entities.size() < 10) {
    tile->world->summon(new Pig(tile->world, vec3(gx, gy+5, gz)));
    cout << "pig spawn " << endl;
  }
  
  if (tile->world->difficulty > 0) {
    if ((value == blocks->names["snow"]) and tile->entities.size() < 10 and above != nullptr and above->get() == 0
     and above->get_pix()->sunlight == lightmax and tile->world->sunlight < 0.1) {
      cout << "skeleton spawned" << endl;
      tile->world->summon(new Skeleton(tile->world, vec3(gx, gy+5, gz)));
    }
  }
}

void Pixel::tick() {
	const ivec3 dir_array[6] = {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
  int gx, gy, gz;
  global_position(&gx, &gy, &gz);
  cout << "tick " << group << ' ' << int(value) << ' ' << ' ' << gx << ' ' << gy << ' ' << gz << ' ' << scale << endl;
  if (group != nullptr and group->blockval != value) {
    group->del(ivec3(gx, gy, gz));
    group = nullptr;
  }
  if (value != 0 and group == nullptr) {
    bool placed = false;
    for (ivec3 dir : dir_array) {
      Block* block = tile->world->get_global(gx + dir.x*scale, gy + dir.y*scale, gz + dir.z*scale, scale);
      if (block != nullptr) {
        for (Pixel* pix : block->iter_side(-dir)) {
          if (pix->group != nullptr) {
            if (pix->group->spread_to_fill(ivec3(gx, gy, gz))) {
              placed = true;
              break;
            }
          }
        }
      }
      if (placed) break;
    }
    if (!placed) {
      group = new UnlinkedGroup(tile->world);
      group->spread_to_fill(ivec3(gx, gy, gz));
    }
  }
  // if (group != nullptr and value == 0) {
  //   group->del(ivec3(gx, gy, gz));
  // }
  return;
  if (tile == nullptr) {
    cout << "err" << endl;
  }
  //cout << "START PIX tick --------------------------";
  //print (ivec3(gx, gy, gz));
  /*if (false) {// and physicsgroup == nullptr) {
    BlockGroup* group = BlockGroup::make_group(value, tile->world, ivec3(gx, gy, gz));
    //cout << group->block_poses.size() << " size!" << endl;
    if (group->block_poses.size() > 0) {
      if (!group->consts[4] and group->groupname != "water-group") {
        // for (int i = 0; i < 6; i ++) {
        //   cout << group->consts[i] << ' ';
        // } cout << endl;
        // cout << "copying " << group->block_poses.size() << endl;
        group->copy_to_block();
        // cout << "erasing" << endl;
        group->erase_from_world();
        // cout << "done" << endl;
        tile->block_entities.push_back(new FallingBlockEntity(tile->world, group));
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
  }*/
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
    
    //cout << gx << ' ' << gy << ' ' << gz << endl;
    //cout << render_index << endl;
    //cout << "render_update() " << render_index.first << ' ' << render_index.second << endl;
    //lightlevel = -1;
    //calculate_lightlevel();
        //world->glvecs.del(render_index);
        //render_index = pair<int,int>(-1,0);
        //render_flag = true;
        set_render_flag();
        set_light_flag();
        // if (tile != nullptr) {
        //   tile->world->block_update(gx, gy, gz);
        // }
        //cout << render_index.first << ' ' << render_index.second << " x" << endl;
        //render_index.first = -(render_index.first+2);
}

void Pixel::set_all_render_flags() {
  render_flag = true;
}

void Pixel::del(bool remove_faces) {
    int gx, gy, gz;
    global_position(&gx, &gy, &gz);
    if (remove_faces and tile != nullptr and render_index.first != -1) {
      if (render_transparent) {
        tile->world->transparent_glvecs.del(render_index);
      } else {
        tile->world->glvecs.del(render_index);
      }
    }
    // if (physicsgroup != nullptr and !physicsgroup->persistant()) {
    //   physicsgroup->block_poses.erase(ivec3(gx, gy, gz));
    //   if (physicsgroup->block_poses.size() == 0) {
    //     delete physicsgroup;
    //   }
    // }
}

void Pixel::lighting_update() {
  if (light_flag) {
    light_flag = false;
    unordered_set<ivec3,ivec3_hash> poses;
    unordered_set<ivec3,ivec3_hash> next_poses;
    calculate_blocklight(poses);
    while (poses.size() > 0) {
      for (ivec3 pos : poses) {
        Block* b =tile->world->get_global(pos.x, pos.y, pos.z, 1);
        if (b != nullptr) b->get_pix()->calculate_blocklight(next_poses);
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
        Block* b =tile->world->get_global(pos.x, pos.y, pos.z, 1);
        if (b != nullptr) {
          b->get_pix()->calculate_sunlight(next_poses);
          if (b->get_pix()->tile != tile) {
            cout << "err!!!! light update cross tiles " << ' ' << tile->pos.x << ' ' << tile->pos.y << ' ' << tile->pos.z << ' ';
            print(b->get_pix()->tile->pos);
          }
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
    if (total > 100) {
      //cout << total << ' ' << max_size << endl;
    }
  }
}

void Pixel::reset_lightlevel() {
  lightsource = -1;
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
  if (tile == nullptr or (value != 0 and !blocks->blocks[value]->transparent)) {
    return;
  }
  const int decrement = 1;
  int gx, gy, gz;
  global_position(&gx, &gy, &gz);
  const ivec3 dirs[] = {{-1,0,0}, {0,-1,0}, {0,0,-1}, {1,0,0}, {0,1,0}, {0,0,1}};
  int oldsunlight = sunlight;
  sunlight = 0;
  for (ivec3 dir : dirs) {
    int newdec = dir.y == 1 ? 0 : decrement;
    int oppdec = dir.y == -1 ? 0 : decrement;
    Block* block = tile->world->get_global(gx+dir.x*scale, gy+dir.y*scale, gz+dir.z*scale, scale);
    if (block != nullptr and !block->get_pix()->tile->fully_loaded) {
      block == nullptr;
    }
    if (block != nullptr) {
      for (Pixel* pix : block->iter_side(-dir)) {
        if (pix->tile != tile and dir.y == 1 and pix->tile->lightflag) {
          sunlight = lightmax;
        } else if (pix->sunlight - newdec*pix->scale > sunlight and pix->sunlight + oppdec*scale != oldsunlight) {
          sunlight = pix->sunlight - newdec*pix->scale;
        }
      }
    } else if (dir.y == 1) {
      sunlight = lightmax;
    }
  }
  
  bool changed = oldsunlight != sunlight;
  for (ivec3 dir : dirs) {
    int newdec = dir.y == -1 ? 0 : decrement;
    Block* block = tile->world->get_global(gx+dir.x*scale, gy+dir.y*scale, gz+dir.z*scale, scale);
    if (block != nullptr and !block->get_pix()->tile->fully_loaded) {
      block == nullptr;
    }
    if (block != nullptr) {
      for (Pixel* pix : block->iter_side(-dir)) {
        if (pix->sunlight < sunlight - newdec*scale or (pix->sunlight + newdec*scale == oldsunlight and sunlight < oldsunlight)) {
          if (tile == pix->tile) {
            ivec3 pos;
            pix->global_position(&pos.x, &pos.y, &pos.z);
            next_poses.emplace(pos);
          } else {
            pix->set_light_flag();
          }
        }
        if (changed) pix->set_render_flag();
      }
    }
  }
}

void Pixel::calculate_blocklight(unordered_set<ivec3,ivec3_hash>& next_poses) {
  if (tile == nullptr or (value != 0 and !blocks->blocks[value]->transparent)) {
    return;
  }
  const int decrement = 1;
  int gx, gy, gz;
  global_position(&gx, &gy, &gz);
  const ivec3 dirs[] = {{-1,0,0}, {0,-1,0}, {0,0,-1}, {1,0,0}, {0,1,0}, {0,0,1}};
  int oldblocklight = blocklight;
  blocklight = entitylight;
  int oldsource = lightsource;
  lightsource = -1;
  int index = 0;
  for (ivec3 dir : dirs) {
    Block* block = tile->world->get_global(gx+dir.x*scale, gy+dir.y*scale, gz+dir.z*scale, scale);
    if (block != nullptr) {
      int inverse_index = index<3 ? index + 3 : index - 3;
      for (Pixel* pix : block->iter_side(-dir)) {
        if (pix->blocklight - decrement*pix->scale > blocklight and pix->lightsource != inverse_index) {
          blocklight = pix->blocklight - decrement*pix->scale;
          lightsource = index;
        }
      }
    }
    index ++;
  }
  
  bool changed = oldblocklight != blocklight;
  index = 0;
  for (ivec3 dir : dirs) {
    Block* block = tile->world->get_global(gx+dir.x*scale, gy+dir.y*scale, gz+dir.z*scale, scale);
    if (block != nullptr) {
      int inverse_index = index<3 ? index + 3 : index - 3;
      for (Pixel* pix : block->iter_side(-dir)) {
        if (pix->blocklight < blocklight - decrement*scale or (pix->lightsource == inverse_index and blocklight < oldblocklight)) {
          ivec3 pos;
          pix->global_position(&pos.x, &pos.y, &pos.z);
          next_poses.emplace(pos);
        }
        if (changed) pix->set_render_flag();
      }
    }
    index ++;
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

void Pixel::render(RenderVecs* allvecs, RenderVecs* transvecs, Collider* collider, int gx, int gy, int gz, int depth, bool faces[6], bool render_null, bool yield) {
    if (!render_flag) {
        return;
    }
    
    MemVecs vecs;
    
    int newscale = scale;
    if (scale < depth) {
      newscale = depth;
    } else {
      gx += px*scale;
      gy += py*scale;
      gz += pz*scale;
    }
    //cout << "render" << gx << ' ' << gy << ' ' << gz << ' '<< scale << endl;
    GLfloat x = gx;
    GLfloat y = gy;
    GLfloat z = gz;
    render_flag = false;
    
    if (value == 0 or depth < 0) {
      if (render_index.first > -1) {
        if (render_transparent) {
          transvecs->del(render_index);
        } else {
          allvecs->del(render_index);
        }
        render_index = pair<int,int>(-1, 0);
      }
      return;
    } else {
      
      if (yield) {
        std::this_thread::yield();
      }
      
      BlockData* blockdata = blocks->blocks[value];
      
      //cout << x << ' ' << y << ' ' << z << "rendering call" << endl;
      GLfloat uv_start = 0.0f;// + (1/32.0f*(int)value);
      GLfloat uv_end = uv_start + 1.0f;///32.0f;
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
      
      const GLfloat uvmax = scale;
      // /char mat = tex_index;
      /*GLfloat new_uvs[] = {
          0.0f, 1.0f * newscale/minscale,
          1.0f * newscale/minscale, 1.0f * newscale/minscale,
          1.0f * newscale/minscale, 0.0f,
          1.0f * newscale/minscale, 0.0f,
          0.0f, 0.0f,
          0.0f, 1.0f * newscale/minscale
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
      
      
      
      
      block = collider->get_global(gx, gy, gz-newscale, newscale);
      if (faces[5] and ((block != nullptr and block->is_air(0, 0, 1, value)) or (block == nullptr and render_null))) {
          GLfloat new_uvs[] = {
              0.0f, uvmax,
              uvmax, uvmax,
              uvmax, 0.0f,
              uvmax, 0.0f,
              0.0f, 0.0f,
              0.0f, uvmax
          };
          rotate_uv(new_uvs,dirs[5]);
          GLfloat face[] = {
              x + newscale, y, z,
              x, y, z,
              x, y + newscale, z,
              x, y + newscale, z,
              x + newscale, y + newscale, z,
              x + newscale, y, z
          };
          float faceblocklight = 0, facesunlight = 1;
          if (block != nullptr) {
            faceblocklight = block->get_blocklight(0,0,1) / lightmax;
            facesunlight = block->get_sunlight(0,0,1) / lightmax;
          }
          vecs.add_face(face, new_uvs, 0.7f * facesunlight, faceblocklight, minscale, mat[5]);//0.4f
      }
      
      block = collider->get_global(gx, gy-newscale, gz, newscale);
      if (faces[4] and ((block != nullptr and block->is_air(0, 1, 0, value)) or (block == nullptr and render_null))) {
          GLfloat new_uvs[] = {
              0.0f, uvmax,
              uvmax, uvmax,
              uvmax, 0.0f,
              uvmax, 0.0f,
              0.0f, 0.0f,
              0.0f, uvmax
          };
          rotate_uv(new_uvs,dirs[4]);
          GLfloat face[] = {
              x, y, z + newscale,
              x, y, z,
              x + newscale, y, z,
              x + newscale, y, z,
              x + newscale, y, z + newscale,
              x, y, z + newscale,
          };
          float faceblocklight = 0, facesunlight = 1;
          if (block != nullptr) {
            faceblocklight = block->get_blocklight(0, 1, 0) / lightmax;
            facesunlight = block->get_sunlight(0, 1, 0) / lightmax;
          }
          vecs.add_face(face, new_uvs, 0.5f * facesunlight, faceblocklight, minscale, mat[4]);
      }
      
      block = collider->get_global(gx-newscale, gy, gz, newscale);
      if (faces[3] and ((block != nullptr and block->is_air(1, 0, 0, value)) or (block == nullptr and render_null))) {
          GLfloat new_uvs[] = {
              0.0f, uvmax,
              uvmax, uvmax,
              uvmax, 0.0f,
              uvmax, 0.0f,
              0.0f, 0.0f,
              0.0f, uvmax
          };
          rotate_uv(new_uvs,dirs[3]);
          GLfloat face[] = {
              x, y, z,
              x, y, z + newscale,
              x, y + newscale, z + newscale,
              x, y + newscale, z + newscale,
              x, y + newscale, z,
              x, y, z
          };
          float faceblocklight = 0, facesunlight = 1;
          if (block != nullptr) {
            faceblocklight = block->get_blocklight(1, 0, 0) / lightmax;
            facesunlight = block->get_sunlight(1, 0, 0) / lightmax;
          }
          vecs.add_face(face, new_uvs, 0.7f * facesunlight, faceblocklight, minscale, mat[3]);
      }
      
      block = collider->get_global(gx, gy, gz+newscale, newscale);
      if (faces[2] and ((block != nullptr and block->is_air(0, 0, -1, value)) or (block == nullptr and render_null))) {
          GLfloat new_uvs[] = {
              0.0f, uvmax,
              uvmax, uvmax,
              uvmax, 0.0f,
              uvmax, 0.0f,
              0.0f, 0.0f,
              0.0f, uvmax
          };
          rotate_uv(new_uvs,dirs[2]);
          GLfloat face[] = {
              x, y, z + newscale,
              x + newscale, y, z + newscale,
              x + newscale, y + newscale, z + newscale,
              x + newscale, y + newscale, z + newscale,
              x, y + newscale, z + newscale,
              x, y, z + newscale
          };
          float faceblocklight = 0, facesunlight = 1;
          if (block != nullptr) {
            faceblocklight = block->get_blocklight(0, 0, -1) / lightmax;
            facesunlight = block->get_sunlight(0, 0, -1) / lightmax;
          }
          vecs.add_face(face, new_uvs, 0.7f * facesunlight, faceblocklight, minscale, mat[2]);
      }
      
      block = collider->get_global(gx, gy+newscale, gz, newscale);
      if (faces[1] and ((block != nullptr and block->is_air(0, -1, 0, value)) or (block == nullptr and render_null))) {
          GLfloat new_uvs[] = {
              0.0f, uvmax,
              uvmax, uvmax,
              uvmax, 0.0f,
              uvmax, 0.0f,
              0.0f, 0.0f,
              0.0f, uvmax
          };
          rotate_uv(new_uvs,dirs[1]);
          GLfloat face[] = {
            
              x, y + newscale, z,
              x, y + newscale, z + newscale,
              x + newscale, y + newscale, z + newscale,
              x + newscale, y + newscale, z + newscale,
              x + newscale, y + newscale, z,
              x, y + newscale, z,
          };
          float faceblocklight = 0, facesunlight = 1;
          if (block != nullptr) {
            faceblocklight = block->get_blocklight(0, -1, 0) / lightmax;
            facesunlight = block->get_sunlight(0, -1, 0) / lightmax;
          }
          vecs.add_face(face, new_uvs, 0.9f * facesunlight, faceblocklight, minscale, mat[1]);//top
      }
      
      block = collider->get_global(gx+newscale, gy, gz, newscale);
      if (faces[0] and ((block != nullptr and block->is_air(-1, 0, 0, value)) or (block == nullptr and render_null))) {
          GLfloat new_uvs[] = {
              0.0f, uvmax,
              uvmax, uvmax,
              uvmax, 0.0f,
              uvmax, 0.0f,
              0.0f, 0.0f,
              0.0f, uvmax
          };
          rotate_uv(new_uvs,dirs[0]);
          GLfloat face[] = {
              x + newscale, y, z + newscale,
              x + newscale, y, z,
              x + newscale, y + newscale, z,
              x + newscale, y + newscale, z,
              x + newscale, y + newscale, z + newscale,
              x + newscale, y, z + newscale
          };
          float faceblocklight = 0, facesunlight = 1;
          if (block != nullptr) {
            faceblocklight = block->get_blocklight(-1, 0, 0) / lightmax;
            facesunlight = block->get_sunlight(-1, 0, 0) / lightmax;
          }
          vecs.add_face(face, new_uvs, 0.7f * facesunlight, faceblocklight, minscale, mat[0]); //0.4f
      }
      
      if (render_index != pair<int,int>(-1,0)) {
        if (render_transparent) {
          transvecs->del(render_index);
          //cout << "transparent delete " << endl;
        } else {
          allvecs->del(render_index);
          //cout << "normal delete " << endl;
        }
        render_index = pair<int,int>(-1,0);
      }
      if (vecs.num_verts != 0) {
        if (blockdata->transparent) {
          render_index = transvecs->add(&vecs);
          render_transparent = true;
        } else {
          render_index = allvecs->add(&vecs);
          render_transparent = false;
        }
      }
    }
    //cout << render_index << endl;
    //cout << "done rendering" << endl;
    //cout << *num_verts << endl;
    //game->crash(1);
}

Chunk* Pixel::subdivide() {
  if (render_index.first != -1 and tile != nullptr) {
    if (render_transparent) {
      tile->world->transparent_glvecs.del(render_index);
    } else {
      tile->world->glvecs.del(render_index);
    }
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
              pix->set_render_flag();
              pix->set_light_flag();
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
        tile->world->glvecs.del(render_index);
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
                pix->set_render_flag();
                pix->set_light_flag();
                //cout << gx+x*scale/csize << ' ' << gy+y*scale/csize << ' ' << gz+z*scale/csize << endl;
                Chunk* subdivided = pix->resolve();
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

Chunk* Pixel::resolve(bool yield) {
  int gx, gy, gz;
  global_position(&gx, &gy, &gz);
  ivec3 gpos(gx, gy, gz);
  bool solid = true;
  char val = tile->world->loader.gen_func(gpos);
  Chunk* chunk = new Chunk(px, py, pz, scale, parent);
  int newscale = scale/csize;
  if (yield) {
    std::this_thread::yield();
  }
  for (int x = 0; x < csize; x ++) {
    for (int y = 0; y < csize; y ++) {
      for (int z = 0; z < csize; z ++) {
        ivec3 pos(x,y,z);
        char newval = tile->world->loader.gen_func(gpos+pos*newscale);
        Pixel* pix = new Pixel(x, y, z, newval, newscale, chunk, tile);
        solid = solid and val == newval;
        if (newscale > 1) {
          Chunk* result = pix->resolve(yield);
          if (result == nullptr) {
            chunk->blocks[x][y][z] = pix;
          } else {
            solid = false;
            delete pix;
            chunk->blocks[x][y][z] = result;
          }
        } else {
          chunk->blocks[x][y][z] = pix;
        }
      }
    }
  }
  if (solid) {
    chunk->del(false);
    delete chunk;
    return nullptr;
  } else {
    return chunk;
  }
}

void Pixel::save_to_file(ostream& of, bool yield) {
  if (yield) {
    std::this_thread::yield();
  }
  if (value != 0 and direction != blocks->blocks[value]->default_direction) {
    write_pix_val(of, 0b01, direction);
  }
  write_pix_val(of, 0b00, value);
}


/////////////////////////////////////// CLASS CHUNK ///////////////////////////////////////

Chunk::Chunk(int x, int y, int z, int nscale, Chunk* nparent):
    Block(x, y, z, nscale, nparent) {}

bool Chunk::continues() const {
    return true;
}

Block * Chunk::get(int x, int y, int z) const {
    //cout << "chunk get px" << px << " py" << py << " pz" << pz << " s" << scale << " x" << x << " y" << y << " z" << z << endl;
    return blocks[x][y][z];
}

Pixel* Chunk::get_pix() {
    return nullptr;
}

Chunk* Chunk::get_chunk() {
    return this;
}

const Pixel* Chunk::get_pix() const {
    return nullptr;
}

const Chunk* Chunk::get_chunk() const {
    return this;
}

char Chunk::get() const {
    cout << "error: get() called on chunk object" << endl;
    return -1;
}

void Chunk::lighting_update() {
  if (light_flag) {
    light_flag = false;
    for (int x = 0; x < csize; x ++) {
        for (int y = csize-1; y >= 0; y --) {
            for (int z = 0; z < csize; z ++) {
                blocks[x][y][z]->lighting_update();
            }
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
          game->crash(8237948232222);
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


void Chunk::render(RenderVecs* vecs, RenderVecs* transvecs, Collider* collider, int gx, int gy, int gz, int depth, bool faces[6], bool render_null, bool yield) {
  if (render_flag) {
    render_flag = false;
    if (depth < scale) {
      for (int x = 0; x < csize; x ++) {
        for (int y = 0; y < csize; y ++) {
          for (int z = 0; z < csize; z ++) {
            blocks[x][y][z]->render(vecs, transvecs, collider, gx + px*scale, gy + py*scale, gz + pz*scale, depth, faces, render_null, yield);
          }
        }
      }
    } else {
      bool chosen = false;
      for (int y = csize-1; y >= 0; y --) {
        for (int x = 0; x < csize; x ++) {
          for (int z = 0; z < csize; z ++) {
            if (!chosen and !blocks[x][y][z]->is_air(0,0,0)) {
              if (depth == scale) {
                blocks[x][y][z]->render(vecs, transvecs, collider, gx + px*scale, gy + py*scale, gz + pz*scale, depth, faces, render_null, yield);
              } else {
                blocks[x][y][z]->render(vecs, transvecs, collider, gx, gy, gz, depth, faces, render_null, yield);
              }
              chosen = true;
            } else {
              blocks[x][y][z]->render(vecs, transvecs, collider, gx + px*scale, gy + py*scale, gz + pz*scale, -69, faces ,render_null, yield);
            }
          }
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

void Chunk::set_all_render_flags() {
    render_flag = true;
    for (int x = 0; x < csize; x ++) {
        for (int y = 0; y < csize; y ++) {
            for (int z = 0; z < csize; z ++) {
                blocks[x][y][z]->set_all_render_flags();
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

void Chunk::save_to_file(ostream& of, bool yield) {
    of << char(0b11000000);
    for (int x = 0; x < csize; x ++) {
        for (int y = 0; y < csize; y ++) {
            for (int z = 0; z < csize; z ++) {
                blocks[x][y][z]->save_to_file(of, yield);
            }
        }
    }
}













BlockContainer::BlockContainer(Block* b): block(b) {
	
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






Pixel* BlockIter::iterator::operator*() {
  return pix;
}

BlockIter::iterator BlockIter::iterator::operator++() {
  Block* block = pix;
  while (block->parent != parent->base->parent and block->px == parent->end_pos.x and block->py == parent->end_pos.y and block->pz == parent->end_pos.z) {
    block = block->parent;
  }
  if (block->parent == parent->base->parent) {
    pix = nullptr;
    return *this;
  }
  ivec3 pos(block->px, block->py, block->pz);
  pos = parent->increment_func(pos, parent->start_pos, parent->end_pos);
  block = block->parent->blocks[pos.x][pos.y][pos.z];
  while (block->continues()) {
    block = block->get_chunk()->blocks[parent->start_pos.x][parent->start_pos.y][parent->start_pos.z];
  }
  pix = block->get_pix();
  return *this;
}

BlockIter::iterator BlockIter::begin() {
  Block* block = base;
  while (block->continues()) {
    block = block->get_chunk()->blocks[start_pos.x][start_pos.y][start_pos.z];
  }
  return {this, block->get_pix()};
}

BlockIter::iterator BlockIter::end() {
  return {this, nullptr};
}

bool operator!=(const BlockIter::iterator& iter1, const BlockIter::iterator& iter2) {
  return iter1.parent != iter2.parent or iter1.pix != iter2.pix;
}








const Pixel* ConstBlockIter::iterator::operator*() {
  return pix;
}

ConstBlockIter::iterator ConstBlockIter::iterator::operator++() {
  const Block* block = pix;
  while (block->parent != parent->base->parent and block->px == parent->end_pos.x and block->py == parent->end_pos.y and block->pz == parent->end_pos.z) {
    block = block->parent;
  }
  if (block->parent == parent->base->parent) {
    pix = nullptr;
    return *this;
  }
  ivec3 pos(block->px, block->py, block->pz);
  pos = parent->increment_func(pos, parent->start_pos, parent->end_pos);
  block = block->parent->blocks[pos.x][pos.y][pos.z];
  while (block->continues()) {
    block = block->get_chunk()->blocks[parent->start_pos.x][parent->start_pos.y][parent->start_pos.z];
  }
  pix = block->get_pix();
  return *this;
}

ConstBlockIter::iterator ConstBlockIter::begin() {
  const Block* block = base;
  while (block->continues()) {
    block = block->get_chunk()->blocks[start_pos.x][start_pos.y][start_pos.z];
  }
  return {this, block->get_pix()};
}

ConstBlockIter::iterator ConstBlockIter::end() {
  return {this, nullptr};
}

bool operator!=(const ConstBlockIter::iterator& iter1, const ConstBlockIter::iterator& iter2) {
  return iter1.parent != iter2.parent or iter1.pix != iter2.pix;
}

#endif
