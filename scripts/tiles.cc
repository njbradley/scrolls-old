#ifndef TILES
#define TILES

#include "tiles.h"

#include "blocks.h"
#include "blockiter.h"
#include "blockdata.h"
#include "blockgroups.h"
#include "mobs.h"
#include "world.h"
#include "generative.h"

//#include <ZipLib/ZipFile.h>

void Tile::update_lighting() {
  
  for (int x = 0; x < chunksize; x ++) {
    for (int z = 0; z < chunksize; z ++) {
      int y = chunksize-1;
      while (y >= 0 and chunk->get_global(x,y,z,1)->pixel->value == 0) {
        chunk->get_global(x,y,z,1)->pixel->sunlight = lightmax;
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
          pix->parbl->set_render_flag();
          pix->parbl->set_light_flag();
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
              pix->parbl->set_render_flag();
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
      if (!world->player->spectator and optimized_render) {
        vec3 playpos = world->player->position;// + float(ivec3_hash()(pos) % (chunksize/3) - chunksize/6);
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
          chunk->set_all_render_flags();
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
  path << "saves/" << world->name << "/chunks/" << pos.x << "x" << pos.y << "y" << pos.z << "z.dat";
  //cout << "saving '" << path.str() << "' to file\n";
  if (!world->generation) {
    ifstream ifile(path.str());
    if (!ifile.good()) {
      return;
    }
  }
  ofstream ofile(path.str(), std::ios::binary);
  
  chunk->to_file(ofile);
  ofile << endl << entities.size() << endl;
  for (DisplayEntity* e : entities) {
    //cout << "entity " << e << " saving pos:";
    //print(e->position);
    e->to_file(ofile);
  }
  ofile << block_entities.size() << endl;
  for (FallingBlockEntity* e : block_entities) {
    e->to_file(ofile);
  }
}

void Tile::timestep() {
  if (deleting) {
    return;
  }
  // ERR: started deleting while this function was running
  for (int i = entities.size()-1; i >= 0; i --) {
    //cout << i << endl;
    //int dist = glm::length(world->player->position - entities[i]->position) / 4;
      entities[i]->timestep();
      // if (timestep_clock%500 == 0) {
      //   cout << "timestep entity " << entities[i] << " pos ";
      //   print (entities[i]->position);
      // }
      if (entities[i]->alive) {
        vec3 epos = entities[i]->position;
        ivec3 chunk = ivec3(epos)/world->chunksize - ivec3(epos.x<0,epos.y<0,epos.z<0);
        if (chunk != pos) {
          //cout << "moving " << chunk.x << ' ' << chunk.y << ' ' << chunk.z << "   " << pos.x << ' ' << pos.y << ' ' << pos.z << endl;
          Tile* tile = world->tileat(chunk);
          if (tile != nullptr) {
            tile->entities.push_back(entities[i]);
          } else {
            // cout << "deleting entity " << entities[i] << endl;
            delete entities[i];
          }
          entities.erase(entities.begin()+i);
        }
      } else {
        //entities[i]->die();
        delete entities[i];
        entities.erase(entities.begin()+i);
      }
  }
  
  for (int i = block_entities.size()-1; i >= 0; i --) {
    block_entities[i]->timestep();
    if (block_entities[i]->alive) {
      vec3 epos = block_entities[i]->position;
      ivec3 chunk = ivec3(epos)/world->chunksize - ivec3(epos.x<0,epos.y<0,epos.z<0);
      if (chunk != pos) {
        Tile* tile = world->tileat(chunk);
        if (tile != nullptr) {
          tile->block_entities.push_back(block_entities[i]);
        } else {
          delete block_entities[i];
        }
        block_entities.erase(block_entities.begin()+i);
      }
    } else {
      delete block_entities[i];
      block_entities.erase(block_entities.begin()+i);
    }
  }
}

void Tile::drop_ticks() {
  for (DisplayEntity* entity : entities) {
    entity->drop_ticks();
  }
  for (FallingBlockEntity* e : block_entities) {
    e->drop_ticks();
  }
}


Tile::Tile(ivec3 newpos, World* nworld): pos(newpos), world(nworld), chunksize(nworld->chunksize), deleting(false) {
	//if (writelock.try_lock_for(std::chrono::seconds(1))) {
    // dfile << "ntile ";
    if (world == nullptr) {
      cout << "error world is null" << endl;
      exit(1);
    }
    stringstream path;
  	path << "saves/" << world->name << "/chunks/" << pos.x << "x" << pos.y << "y" << pos.z << "z.dat";
  	ifstream ifile(path.str(), std::ios::binary);
  	if (!ifile.good()) {
  			//cout << "generating '" << path.str() << "'\n";
        // dfile << "NTILE" << endl << "gen ";
  			//chunk = generate(pos);
        generate_chunk(pos);
        ifile.open(path.str(), std::ios::binary);
        // dfile << "GEN " << endl;
  			//return load_chunk(pos);
  			//chunk = generate(pos);
  	}
    vector<BlockGroup*> groups;
    
    chunk = new Block();
    chunk->set_parent(world, pos, chunksize);
    chunk->from_file(ifile);
    
    const ivec3 dir_array[] = {{-1,0,0}, {0,-1,0}, {0,0,-1}, {1,0,0}, {0,1,0}, {0,0,1}};
    
    int size;
    ifile >> size;
    for (int i = 0; i < size; i ++) {
      entities.push_back(Mob::from_file(world, ifile));
    }
    ifile >> size;
    for (int i = 0; i < size; i ++) {
      block_entities.push_back(new FallingBlockEntity(world, ifile));
    }
    done_reading = true;
  	//render_chunk_vectors(pos);
  	//render(&world->glvecs);
  //   writelock.unlock();
  // } else {
  //   cout << "tile locked when created?" << endl;
  //   game->crash(1188188839499);
  // }
}

Tile::Tile(ivec3 position, World* nworld, istream& ifile): pos(position), world(nworld), chunksize(nworld->chunksize), deleting(false) {
  chunk = new Block();
  chunk->set_parent(world, position, chunksize);
  chunk->from_file(ifile);
  
  const ivec3 dir_array[] = {{-1,0,0}, {0,-1,0}, {0,0,-1}, {1,0,0}, {0,1,0}, {0,0,1}};
  
  int size;
  ifile >> size;
  for (int i = 0; i < size; i ++) {
    entities.push_back(Mob::from_file(world, ifile));
  }
  ifile >> size;
  for (int i = 0; i < size; i ++) {
    block_entities.push_back(new FallingBlockEntity(world, ifile));
  }
  done_reading = true;
}


Tile::~Tile() {
  //cout << "start of deleting tile ";
  deletelock.lock();
  // dfile << "del " << endl;
  deleting = true;
  for (DisplayEntity* entity : entities) {
    // cout << "deleting tile del entitiy " << entity << endl;
    delete entity;
  }
  delete chunk;
  // dfile << "DEL" << endl;
  deletelock.unlock();
}


void Tile::generate_chunk(ivec3 pos) {
  //generative_setup_chunk(pos.first, pos.second);
  stringstream path;
  path << "saves/" << world->name << "/chunks/" << pos.x << "x" << pos.y << "y" << pos.z << "z.dat";
  ofstream of(path.str(), std::ios::binary);
  char val = gen_block(of, pos.x*World::chunksize, pos.y*World::chunksize, pos.z*World::chunksize, chunksize);
  of << endl << 0 << endl << 0 << endl;
  //cout << (int)val << int(char(0xff)) << endl;
}

char Tile::gen_block(ostream& ofile, int gx, int gy, int gz, int scale) {
  char val = world->loader.gen_func(ivec4(gx, gy, gz, scale));
  // cout << int(val) << ' ' << ivec3(gx,gy,gz) << ' ' << scale << endl;
  if (scale == 1 or val != -1) {
    Block::write_pix_val(ofile, 0b00, val);
    return val;
  } else {
    stringstream ss;
    ss << char(0b11000000);
    char val = gen_block(ss, gx, gy, gz, scale/csize);
    bool all_same = val != -1;
    for (int x = 0; x < csize; x ++) {
      for (int y = 0; y < csize; y ++) {
        for (int z = 0; z < csize; z ++) {
          if (x > 0 or y > 0 or z > 0) {
            char newval = gen_block(ss, gx+x*(scale/csize), gy+y*(scale/csize), gz+z*(scale/csize), scale/csize);
            all_same = all_same and newval == val;
          }
        }
      }
    }
    if (all_same) {
      Block::write_pix_val(ofile, 0b00, val);
      return val;
    } else {
      ofile << ss.rdbuf();
      return -1;
    }
  }
}
/*

char World::gen_block(ostream& ofile, ChunkLoader* loader, int gx, int gy, int gz, int scale) {
  if (scale == 1) {
    char val = loader->gen_func(gx, gy, gz);
    ofile << val;
    return val;
  } else {
    stringstream ss;
    ss << '{';
    char val = gen_block(ss, loader, gx, gy, gz, scale/2);
    bool all_same = val != -1;
    for (int x = 0; x < csize; x ++) {
      for (int y = 0; y < csize; y ++) {
        for (int z = 0; z < csize; z ++) {
          if (x > 0 or y > 0 or z > 0) {
            char newval = gen_block(ss, loader, gx+x*(scale/2), gy+y*(scale/2), gz+z*(scale/2), scale/2);
            all_same = all_same and newval == val;
          }
        }
      }
    }
    if (all_same) {
      ofile << val;
      return -1;
    } else {
      ss << '}';
      ofile << ss.rdbuf();
      return 0xff;
    }
  }
}
*/

#endif
