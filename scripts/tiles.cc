#ifndef TILES
#define TILES

#include "tiles.h"

#include "blocks.h"
#include "blockdata.h"
#include "blockgroups.h"
#include "mobs.h"
#include "world.h"
#include "generative.h"

//#include <ZipLib/ZipFile.h>

Block* Tile::generate(ivec3 pos) {
  Pixel* pix = new Pixel(pos.x,pos.y,pos.z,0,chunksize,nullptr,this);
  if (world->generation) {
    Chunk* chunk = pix->resolve(true);
  	if (chunk == nullptr) {
  		return pix;
  	} else {
  		return chunk;
  	}
  } else {
    return pix;
  }
}

void Tile::update_lighting() {
  
  for (int x = 0; x < chunksize; x ++) {
    for (int z = 0; z < chunksize; z ++) {
      int y = chunksize-1;
      while (y >= 0 and chunk->get_global(x,y,z,1)->get() == 0) {
        chunk->get_global(x,y,z,1)->get_pix()->sunlight = lightmax;
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
        for (Pixel* pix : tile->chunk->iter_side(-dir)) {
          pix->set_render_flag();
          pix->set_light_flag();
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
        //update_lighting();
        const ivec3 dirs[] = {{-1,0,0}, {0,-1,0}, {0,0,-1}, {1,0,0}, {0,1,0}, {0,0,1}};
        for (ivec3 dir : dirs) {
          Tile* tile = world->tileat(pos+dir);
          if (tile != nullptr) {
            for (Pixel* pix : tile->chunk->iter_side(-dir)) {
              pix->set_render_flag();
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
      chunk->render(glvecs, transvecs, world, 0, 0, 0, render_depth, render_faces, false, true);
    }
    changelock.unlock();
    deletelock.unlock_shared();
  }
  // if (!fully_loaded) {
  //   std::terminate();
  // }
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
  vector<BlockGroup*> groups;
  
  chunk->save_to_file(ofile, &groups, true);
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
  
  
  path.str("");
  path << "saves/" << world->name << "/groups/" << pos.x << "x" << pos.y << "y" << pos.z << "z.dat";
  ofstream groupfile(path.str());
  if (groups.size() > 0) {
    for (BlockGroup* group : groups) {
      group->to_file(groupfile, &groups);
    }
  } else {
    if (groupfile.good()) {
      groupfile.close();
      remove(path.str().c_str());
    }
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

void Tile::get_side_groups(unordered_map<int,BlockGroup*>& sidegroups, Tile* tile, ivec3 dir) {
  for (Pixel* tilepix : tile->chunk->iter_side(dir)) {
    if (tilepix->group != nullptr and tilepix->group->final) {
      //cout << "encontra un bloque con grupo"<< endl;
      sidegroups[tilepix->group->id] = tilepix->group;
    }
  }
}

void Tile::fix_side_groups(unordered_map<int,BlockGroup*>& sidegroups) {
  unordered_set<BlockGroup*> to_del;
  for (Pixel* pix : chunk->iter()) {
    if (pix->group != nullptr and pix->group->final) {
      if (sidegroups.find(pix->group->id) != sidegroups.end()) {
        if (to_del.count(pix->group) == 0) {
          sidegroups[pix->group->id]->merge_copy(pix->group);
        }
        to_del.emplace(pix->group);
        pix->group = sidegroups[pix->group->id];
      } else {
        for (int i = 0; i < pix->group->connections.size(); i ++) {
          if (sidegroups.find(pix->group->connections[i].globalindex) != sidegroups.end()) {
            //cout << "reparando los conneciones del grupo"<< endl;
            BlockGroup* connected_group = sidegroups[pix->group->connections[i].globalindex];
            //cout << "nuevo grupo "<< connected_group << endl;
            //cout << "old group" << pix->group->connections[i].group << endl;
            pix->group->connections[i].group = connected_group;
            
            for (int j = 0; j < connected_group->connections.size(); j ++) {
              if (connected_group->connections[j].globalindex == pix->group->id) {
                connected_group->connections[j].group = pix->group;
                break;
              }
            }
            break;
          }
        }
      }
    }
  }
  
  for (BlockGroup* delgroup : to_del) {
    delete delgroup;
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
    path.str("");
  	path << "saves/" << world->name << "/groups/" << pos.x << "x" << pos.y << "y" << pos.z << "z.dat";
    ifstream groupfile(path.str());
    vector<BlockGroup*> groups;
    if (groupfile.good()) {
      while (!groupfile.eof()) {
        groups.push_back(new BlockGroup(world, groupfile));
        while (!groupfile.eof() and (groupfile.peek() == ' ' or groupfile.peek() == '\n')) {
          groupfile.get();
        }
      }
    }
    for (BlockGroup* group : groups) {
      group->link(&groups);
    }
    
    chunk = Block::from_file(ifile, pos.x, pos.y, pos.z, chunksize, nullptr, this, &groups);
    
    const ivec3 dir_array[] = {{-1,0,0}, {0,-1,0}, {0,0,-1}, {1,0,0}, {0,1,0}, {0,0,1}};
    // Este busca para grupos duplicados, y lo repara con el grupo de otro tile.
    if (groups.size() > 0) {
      unordered_map<int,BlockGroup*> sidegroups;
      cout << "start loop "<< endl;
      for (ivec3 dir : dir_array) {
        Tile* tile = world->tileat(pos + dir);
        if (tile != nullptr) {
          size_t myhash = hash4(23489239, pos.x, pos.y, pos.z, 82943);
          ivec3 dirpos = dir + pos;
          size_t otherhash = hash4(23489239, dirpos.x, dirpos.y, dirpos.z, 82943);
          cout << pos << ' ' << pos + dir << ' ' << myhash << ' ' << otherhash << endl;
          // get_side_groups(sidegroups, tile, -dir);
          if (myhash > otherhash) {
            cout << "myhash is greater" << endl;
            get_side_groups(sidegroups, tile, -dir);
          } else if (otherhash > myhash and tile->done_reading) {
            cout << "otherhash is greater " << endl;
            unordered_map<int,BlockGroup*> othersidegroups;
            get_side_groups(othersidegroups, this, dir);
            if (othersidegroups.size() > 0) {
              tile->fix_side_groups(othersidegroups);
            }
          } else {
            cout << "mismo tiempo o mismo hash"<< endl;
          }
        }
      }
      cout << "end loop" << endl;
      
      if (sidegroups.size() > 0) {
        fix_side_groups(sidegroups);
      }
    }
    
    /*
            for (Pixel* mypix : tilepix->iter_touching(world)) {
              cout << "bloque "<< endl;
              if (mypix->tile == this and mypix->group != nullptr) {
                cout << "bloque de mi"<< endl;
                if (mypix->group->id == tilepix->group->id) {
                  cout << "Esta reparando el grupo"<< endl;
                  BlockGroup* oldgroup = mypix->group;
                  for (Pixel* pix : mypix->iter_group()) {
                    pix->group = tilepix->group;
                  }
                  delete oldgroup;
                } else {
                  for (int i = 0; i < mypix->group->connections.size(); i ++) {
                    if (mypix->group->connections[i].globalindex == tilepix->group->id) {
                      cout << "Esta reparando los conneciones del grupo"<< endl;
                      mypix->group->connections[i].group = tilepix->group;
                      
                      for (int j = 0; j < tilepix->group->connections.size(); j ++) {
                        if (tilepix->group->connections[j].globalindex == mypix->group->id) {
                          tilepix->group->connections[j].group = mypix->group;
                          break;
                        }
                      }
                      break;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }*/
    
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


void Tile::del(bool remove_faces) {
  //cout << "start of deleting tile ";
  deletelock.lock();
  // dfile << "del " << endl;
  deleting = true;
  for (DisplayEntity* entity : entities) {
    // cout << "deleting tile del entitiy " << entity << endl;
    delete entity;
  }
  chunk->del(remove_faces);
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
  if (scale == 1) {
    char val = world->loader.gen_func(ivec3(gx, gy, gz));
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
