#ifndef TILES
#define TILES

#include "tiles-predef.h"
#include "blocks-predef.h"
#include "blockdata-predef.h"

//#include <ZipLib/ZipFile.h>

Block* Tile::generate(ivec3 pos) {
    cout << chunksize << ' ' << pos.x << ',' << pos.y << ' ' << pos.z << endl;
    Pixel* pix = new Pixel(pos.x,pos.y,pos.z,0,chunksize,nullptr,this);
    Chunk* chunk = pix->resolve();
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
  if (writelock.try_lock_for(std::chrono::seconds(1))) {
    //update_lighting();
    //chunk->calculate_lightlevel();
	   chunk->render(glvecs, 0, 0, 0);
     //cout << chunk->render_flag << endl;
     writelock.unlock();
   }
}

void Tile::save() {
  stringstream path;
  path << "saves/world/chunk" << pos.x << "x" << pos.y << "y" << pos.z << "z.dat";
  cout << "saving '" << path.str() << "' to file\n";
  ofstream of(path.str(), ios::binary);
  chunk->save_to_file(of);
}

void Tile::timestep() {
  for (int i = entities.size()-1; i >= 0; i --) {
    entities[i]->timestep(world);
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
  }
}

Tile::Tile(ivec3 newpos, World* nworld): pos(newpos), world(nworld), chunksize(nworld->chunksize) {
	if (writelock.try_lock_for(std::chrono::seconds(1))) {
    if (pos == ivec3(0,2,15)) {
      entities.push_back(new NamedEntity(pos*chunksize+32, "test"));
  	}
    stringstream path;
  	path << "saves/world/chunk" << pos.x << "x" << pos.y << "y" << pos.z << "z.dat";
  	ifstream ifile(path.str(), ios::binary);
  	if (ifile.good()) {
  			cout << "loading '" << path.str() << "' from file\n";
  			//chunks[pos] = parse_file(&ifile, pos.first, 0, pos.second, chunksize, nullptr);
  			chunk = Block::from_file(ifile, pos.x, pos.y, pos.z, chunksize, nullptr, this);
  	} else {
  			cout << "generating '" << path.str() << "'\n";
  			chunk = generate(pos);
  			//return load_chunk(pos);
  			//chunk = generate(pos);
  	}
  	//render_chunk_vectors(pos);
  	//render(&world->glvecs);
    writelock.unlock();
  } else {
    cout << "tile locked when created?" << endl;
    crash(1188188839499);
  }
}

void Tile::del(bool remove_faces) {
  if (writelock.try_lock_for(std::chrono::seconds(1))) {
	  chunk->del(remove_faces);
    writelock.unlock();
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
