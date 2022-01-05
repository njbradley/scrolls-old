#ifndef TILES
#define TILES

#include "tiles.h"

#include "blocks.h"
#include "blockiter.h"
#include "blockdata.h"
#include "world.h"
#include "entity.h"
#include "player.h"
#include "terrain.h"

//#include <ZipLib/ZipFile.h>

void Tile::update_lighting() {
  
  for (int x = 0; x < chunksize; x ++) {
    for (int z = 0; z < chunksize; z ++) {
      int y = chunksize-1;
      while (y >= 0 and chunk->get_global(ivec3(x,y,z),1)->pixel->value == 0) {
        chunk->get_global(ivec3(x,y,z),1)->pixel->sunlight = lightmax;
        y--;
      }
    }
  }
}

void Tile::lighting_update() {
  const ivec3 dirs[] = {{-1,0,0}, {0,-1,0}, {0,0,-1}, {1,0,0}, {0,1,0}, {0,0,1}};
  chunk->lighting_update();
  if (lightflag) {
    lightflag = false;
    for (ivec3 dir : dirs) {
      Tile* tile = world->tileat(pos+dir);
      if (tile != nullptr) {
        // cout << "side update" << endl;
        for (Pixel* pix : tile->chunk->iter_side(-dir)) {
          pix->parbl->set_flag(Block::RENDER_FLAG | Block::LIGHT_FLAG);
        }
        //tile->lighting_update();
      }
    }
  }
}

void Tile::render(RenderVecs* glvecs, RenderVecs* transvecs) {
  //if (writelock.try_lock_for(std::chrono::seconds(1))) {
  
  if (deletelock.try_lock_shared()) {
    changelock.lock();
    if (lightflag) {
      std::terminate();
        //update_lighting();
        const ivec3 dirs[] = {{-1,0,0}, {0,-1,0}, {0,0,-1}, {1,0,0}, {0,1,0}, {0,0,1}};
        for (ivec3 dir : dirs) {
          Tile* tile = world->tileat(pos+dir);
          if (tile != nullptr) {
            cout << "side update 2" << endl;
            for (Pixel* pix : tile->chunk->iter_side(-dir)) {
              pix->parbl->set_flag(Block::RENDER_FLAG | Block::LIGHT_FLAG);
            }
          }
        }
        // for (int i = 0; i < 10; i ++) {
        //   chunk->calculate_lightlevel();
        // }
      lightflag = false;
    }
    //chunk->calculate_lightlevel();
    if (!deleting) { // ERR: starts deleting during render
      if (world->player != nullptr and !world->player->spectator and optimized_render) {
        vec3 playpos = world->player->box.position;// + float(ivec3_hash()(pos) % (chunksize/3) - chunksize/6);
        ivec3 player_chunk((playpos.x/world->chunksize) - (playpos.x<0),
                           (playpos.y/world->chunksize) - (playpos.y<0),
                           (playpos.z/world->chunksize) - (playpos.z<0));
        double player_dist = glm::length(vec3(player_chunk) - vec3(pos)) / 2;
        int depth = 1;
        while (depth < player_dist) {
          depth *= 2;
        }
        
        bool changed = false;
        
        // if (depth != render_depth) {
        //   render_depth = depth;
        //   changed = true;
        // }
        
        
        float dist = glm::length(vec3(player_chunk) - vec3(pos));
        
        int off = 2;
        
        // if (dist > 4) {
        //   off = 0;
        // } else
        // if (dist > 3) {
        //   off = 1;
        // }
        
        bool faces[] = {
          player_chunk.x > pos.x-off,
          player_chunk.y > pos.y-off,
          player_chunk.z > pos.z-off,
          player_chunk.x < pos.x+off,
          player_chunk.y < pos.y+off,
          player_chunk.z < pos.z+off,
        };
        
        
        for (int i = 0; i < 6; i ++) {
          if (faces[i] != render_faces[i]) {
            render_faces[i] = faces[i];
            changed = true;
          }
        }
        
        if (changed) {
          chunk->set_all_flags(Block::RENDER_FLAG);
        }
      }
      //cout << world << endl;
      chunk->lighting_update();
      chunk->render(glvecs, transvecs, 0xff, false);
    }
    changelock.unlock();
    deletelock.unlock_shared();
  }
  if (!fully_loaded) {
    //cout << 142 << ' ' << pos << endl;
  }
  fully_loaded = true;
}

void Tile::save() {
  stringstream path;
  path << SAVES_PATH << world->name << "/chunks/" << pos.x << "x" << pos.y << "y" << pos.z << "z.dat";
  //cout << "saving '" << path.str() << "' to file\n";
  if (!world->generation) {
    ifstream ifile(path.str());
    if (!ifile.good()) {
      return;
    }
  }
  ofstream ofile(path.str(), std::ios::binary);
  
  chunk->to_file(ofile);
}

void Tile::tick(float curtime, float deltatime) {
  for (FreeBlock* free = allfreeblocks; free != nullptr; free = free->allfreeblocks) {
    free->tick(curtime, deltatime);
  }
}

void Tile::timestep(float curtime, float deltatime) {
  if (deleting or !fully_loaded) {
    return;
  }
  for (FreeBlock* free = allfreeblocks; free != nullptr; free = free->allfreeblocks) {
    free->timestep(curtime, deltatime);
  }
}

void Tile::drop_ticks() {
  
}


Tile::Tile(ivec3 newpos, World* nworld): pos(newpos), world(nworld), chunksize(nworld->chunksize), deleting(false) {
  if (world == nullptr) {
    cout << "error world is null" << endl;
    exit(1);
  }
  stringstream path;
	path << SAVES_PATH << world->name << "/chunks/" << pos.x << "x" << pos.y << "y" << pos.z << "z.dat";
	ifstream ifile(path.str(), std::ios::binary);
	if (!ifile.good()) {
			//cout << "generating '" << path.str() << "'\n";
      // dfile << "NTILE" << endl << "gen ";
			//chunk = generate(pos);
      chunk = world->terrainloader.generate_chunk(pos);
      chunk->set_parent(this, pos, chunksize);
      // dfile << "GEN " << endl;
			//return load_chunk(pos);
			//chunk = generate(pos);
	} else {
    
    chunk = new Block();
    chunk->set_parent(this, pos, chunksize);
    chunk->from_file(ifile);
  }
}


Tile::~Tile() {
  //cout << "start of deleting tile ";
  deletelock.lock();
  // dfile << "del " << endl;
  deleting = true;
  delete chunk;
  // dfile << "DEL" << endl;
  deletelock.unlock();
}

Block* Tile::get_global(ivec3 pos, int scale) {
  return world->get_global(pos,scale);
}

const Block* Tile::get_global(ivec3 pos, int scale) const {
  return world->get_global(pos,scale);
}

vec3 Tile::get_position() const {
  return pos * World::chunksize;
}

void Tile::block_update(ivec3 pos) {
  world->block_update(pos);
}

void Tile::set_global(ivec3 pos, int w, Blocktype val, int direc, int joints[6]) {
  world->set_global(pos, w, val, direc, joints);
}

void Tile::add_freeblock(FreeBlock* freeblock) {
  freeblock->allfreeblocks = allfreeblocks;
  allfreeblocks = freeblock;
}

void Tile::remove_freeblock(FreeBlock* freeblock) {
  FreeBlock** free = &allfreeblocks;
  while (*free != nullptr) {
    if (*free == freeblock) {
      *free = (*free)->allfreeblocks;
      return;
    }
    free = &(*free)->allfreeblocks;
  }
}

DEFINE_PLUGIN(TileLoader);

#endif
