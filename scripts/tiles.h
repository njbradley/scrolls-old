#ifndef TILES
#define TILES

#include "tiles-predef.h"
#include "blocks-predef.h"
#include "blockdata-predef.h"
#include "mobs-predef.h"

//#include <ZipLib/ZipFile.h>

Block* Tile::generate(ivec3 pos) {
  Pixel* pix = new Pixel(pos.x,pos.y,pos.z,0,chunksize,nullptr,this);
  Chunk* chunk = pix->resolve([=] (ivec3 position) {return world->loader.gen_func(position);});
	if (chunk == nullptr) {
		return pix;
	} else {
		return chunk;
	}
}

void Tile::update_lighting() {
  chunk->all([=] (Pixel* pix) {
    pix->reset_lightlevel();
    //cout << pix->lightlevel << endl;
  });
  for (int x = 0; x < chunksize; x ++) {
    for (int z = 0; z < chunksize; z ++) {
      int y = chunksize-1;
      while (y >= pos.y and chunk->get_global(x,y,z,1)->get() == 0) {
        chunk->get_global(x,y,z,1)->get_pix()->lightlevel = 1;
        y--;
      }
    }
  }
}

void Tile::render(GLVecs* glvecs) {
  //if (writelock.try_lock_for(std::chrono::seconds(1))) {
    //update_lighting();
    //chunk->calculate_lightlevel();
    if (!deleting) {
       //cout << world << endl;
	     chunk->render(glvecs, world, 0, 0, 0);
     }
     // for (int i = entities.size()-1; i >= 0; i --) {
     //   if (!entities[i]->alive) {
     //     delete entities[i];
     //     entities.erase(entities.begin()+i);
     //   }
     // }
     //cout << chunk->render_flag << endl;
  //   writelock.unlock();
   //}
}

void Tile::save() {
  stringstream path;
  path << "saves/" << world->name << "/chunks/" << pos.x << "x" << pos.y << "y" << pos.z << "z.dat";
  //cout << "saving '" << path.str() << "' to file\n";
  ofstream ofile(path.str(), ios::binary);
  chunk->save_to_file(ofile);
  ofile << endl << entities.size() << endl;
  for (DisplayEntity* e : entities) {
    e->to_file(ofile);
  }
  ofile << block_entities.size() << endl;
  for (FallingBlockEntity* e : block_entities) {
    e->to_file(ofile);
  }
}

void Tile::timestep(int timestep_clock) {
  if (deleting) {
    return;
  }
  // ERR: started deleting while this function was running
  for (int i = entities.size()-1; i >= 0; i --) {
    //cout << i << endl;
    //int dist = glm::length(world->player->position - entities[i]->position) / 4;
      entities[i]->timestep();
      if (entities[i]->alive) {
        vec3 epos = entities[i]->position;
        ivec3 chunk = ivec3(epos)/world->chunksize - ivec3(epos.x<0,epos.y<0,epos.z<0);
        if (chunk != pos) {
          if (world->tiles.find(pos) != world->tiles.end()) {
            world->tiles[pos]->entities.push_back(entities[i]);
          } else {
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
        if (world->tiles.find(pos) != world->tiles.end()) {
          world->tiles[pos]->block_entities.push_back(block_entities[i]);
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
    if (pos == ivec3(0,0,0)) {
      //cout << "summoned entity 0923472893472893748293749823749273492734892739482739482739482739487293847283" << endl;
      // vector<DisplayEntity*> limbs {
      //   new NamedEntity(world, vec3(-0.5,-0.5,0.5), vec3(0,0,0), vec3(1,1,1), "pigfoot", vec3(0,0,0)),
      //   new NamedEntity(world, vec3(3.5,-0.5,0.5), vec3(0,0,0), vec3(1,1,1), "pigfoot", vec3(0,0,0)),
      //   new NamedEntity(world, vec3(-0.5,-0.5,2.5), vec3(0,0,0), vec3(1,1,1), "pigfoot", vec3(0,0,0)),
      //   new NamedEntity(world, vec3(3.5,-0.5,2.5), vec3(0,0,0), vec3(1,1,1), "pigfoot", vec3(0,0,0)),
      //
      // };
      entities.push_back(new Skeleton(world, pos*chunksize + 32 + ivec3(0,5,0)));
      //entities.push_back(new NamedEntity(world, pos*chunksize+32 + ivec3(0,5,0), vec3(0,0,0), vec3(2,2,2), "pighead", vec3(0,0,0)));
      // entities.push_back(new NamedEntity(world, pos*chunksize+32 + ivec3(0,5,0), vec3(-1,0,-1), vec3(1,4,1), "skeleton", vec3(-2.5,0,-2.5), limbs));
  	}
    if (world == nullptr) {
      cout << "error world is null" << endl;
      exit(1);
    }
    stringstream path;
  	path << "saves/" << world->name << "/chunks/" << pos.x << "x" << pos.y << "y" << pos.z << "z.dat";
  	ifstream ifile(path.str(), ios::binary);
  	if (ifile.good()) {
  			//cout << "loading '" << path.str() << "' from file\n";
  			//chunks[pos] = parse_file(&ifile, pos.first, 0, pos.second, chunksize, nullptr);
  			chunk = Block::from_file(ifile, pos.x, pos.y, pos.z, chunksize, nullptr, this);
        int size;
        ifile >> size;
        for (int i = 0; i < size; i ++) {
          entities.push_back(Mob::from_file(world, ifile));
        }
        ifile >> size;
        for (int i = 0; i < size; i ++) {
          block_entities.push_back(new FallingBlockEntity(world, ifile));
        }
  	} else {
  			//cout << "generating '" << path.str() << "'\n";
  			chunk = generate(pos);
  			//return load_chunk(pos);
  			//chunk = generate(pos);
  	}
  	//render_chunk_vectors(pos);
  	//render(&world->glvecs);
  //   writelock.unlock();
  // } else {
  //   cout << "tile locked when created?" << endl;
  //   crash(1188188839499);
  // }
}


void Tile::del(bool remove_faces) {
  //cout << "start of deleting tile ";
  if (writelock.try_lock_for(std::chrono::seconds(1))) {
    deleting = true;
    for (DisplayEntity* entity : entities) {
      delete entity;
    }
	  chunk->del(remove_faces);
    delete chunk;
    writelock.unlock();
  } else {
    cout << "ERR: tile not properly deleted" << endl;
    crash(43928739482);
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
