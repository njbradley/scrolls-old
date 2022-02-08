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
#include "debug.h"

//#include <ZipLib/ZipFile.h>


void Tile::lighting_update() {
  const ivec3 dirs[] = {{-1,0,0}, {0,-1,0}, {0,0,-1}, {1,0,0}, {0,1,0}, {0,0,1}};
  // chunk->lighting_update();
  
  if (lightflag) {
    lightflag = false;
    for (ivec3 dir : dirs) {
      BlockView view (world);
      if (view.moveto((pos + dir) * World::chunksize, World::chunksize)) {
        for (BlockView& view : view.iter_side(-dir)) {
          view->set_flag(Block::RENDER_FLAG | Block::LIGHT_FLAG);
        }
      }
    }
  }
}

void Tile::render(RenderVecs* glvecs, RenderVecs* transvecs) {
  if (deletelock.try_lock_shared()) {
    changelock.lock();
    if (!deleting) {
      BlockView view (world);
      if (view.moveto(pos * World::chunksize, World::chunksize)) {
        for (BlockView& view : view.iter()) {
          view->pixel->lighting_update(view);
          view->pixel->render(view, glvecs, transvecs);
        }
      }
    }
    changelock.unlock();
    deletelock.unlock_shared();
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
  // for (FreeBlock* free = allfreeblocks; free != nullptr; free = free->allfreeblocks) {
  //   free->tick(curtime, deltatime);
  // }
}

void Tile::timestep(float curtime, float deltatime) {
  if (deleting or !fully_loaded) {
    return;
  }
  // for (FreeBlock* free = allfreeblocks; free != nullptr; free = free->allfreeblocks) {
  //   free->timestep(curtime, deltatime);
  // }
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
      chunk->set_parent(nullptr);
      // dfile << "GEN " << endl;
			//return load_chunk(pos);
			//chunk = generate(pos);
	} else {
    
    chunk = new Block();
    chunk->set_parent(nullptr);
    chunk->from_file(ifile);
  }
  // debuglines->render(chunk->hitbox(), vec3(1,1,1), "chunk_borders");
}

Tile::Tile(ivec3 newpos, World* nworld, Block* nchunk): pos(newpos), world(nworld), chunksize(nworld->chunksize), deleting(false) {
  chunk = nchunk;
  chunk->set_parent(nullptr);
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

void Tile::set_chunk(Block* nchunk) {
  if (chunk != nullptr) {
    delete chunk;
  }
  chunk = nchunk;
  chunk->set_parent(nullptr);
}

BlockView Tile::get_global(ivec3 pos, int scale) {
  return world->get_global(pos,scale);
}

ConstBlockView Tile::get_global(ivec3 pos, int scale) const {
  return ((const World*)world)->get_global(pos,scale);
}


Block* Tile::get_child(ivec3 npos) {
  if (npos == pos) {
    return chunk;
  } else {
    return world->get_child(npos);
  }
}

const Block* Tile::get_child(ivec3 npos) const {
  if (npos == pos) {
    return chunk;
  } else {
    return world->get_child(npos);
  }
}
void Tile::set_child(ivec3 npos, Block* block) {
  if (npos == pos) {
    chunk = block;
    block->set_parent(nullptr);
  } else {
    world->set_child(npos, block);
  }
}

// void Tile::add_freeblock(FreeBlock* freeblock) {
//   freeblock->allfreeblocks = allfreeblocks;
//   allfreeblocks = freeblock;
// }
//
// void Tile::remove_freeblock(FreeBlock* freeblock) {
//   FreeBlock** free = &allfreeblocks;
//   while (*free != nullptr) {
//     if (*free == freeblock) {
//       *free = (*free)->allfreeblocks;
//       return;
//     }
//     free = &(*free)->allfreeblocks;
//   }
// }

DEFINE_PLUGIN(TileLoader);

#endif
