#ifndef TILES
#define TILES

#include "tiles-predef.h"
#include "blocks-predef.h"
#include "blockdata-predef.h"

//#include <ZipLib/ZipFile.h>

Block* Tile::parse_file(istream& ifile, int px, int py, int pz, int scale, Chunk* parent) {
    char c;
    ifile.read(&c,1);
    if (c == '{') {
        Chunk* chunk = new Chunk(px, py, pz, scale, parent);
        for (int x = 0; x < csize; x ++) {
            for (int y = 0; y < csize; y ++) {
                for (int z = 0; z < csize; z ++) {
                    chunk->blocks[x][y][z] = parse_file(ifile, x, y, z, scale/csize, chunk);
                }
            }
        }
        ifile.read(&c,1);
        if (c != '}') {
            cout << "error, mismatched {} in save file" << endl;
        }
        return (Block*) chunk;
    } else {
        if (c == '~') {
          c = ifile.get();
          BlockExtras* extra = new BlockExtras(ifile);
          Pixel* p = new Pixel(px, py, pz, c, scale, parent, this, extra);
          return (Block*) p;
        } else {
          Pixel* p = new Pixel(px, py, pz, c, scale, parent, this);
          return (Block*) p;
        }
    }
}

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
  for (int x = pos.x*chunksize; x < pos.x*chunksize+chunksize; x ++) {
    for (int z = pos.z*chunksize; z < pos.z*chunksize+chunksize; z ++) {
      int y = pos.y+chunksize-1;
      while (y >= pos.y and world->get(x,y,z) == 0) {
        world->get_global(x,y,z,1)->get_pix()->lightlevel = 1;
        y--;
      }
    }
  }
}

void Tile::render(GLVecs* glvecs) {
	chunk->render(glvecs, 0, 0, 0);
}

void Tile::save() {
  stringstream path;
  path << "saves/world/chunk" << pos.x << "x" << pos.y << "y" << pos.z << "z.dat";
  cout << "saving '" << path.str() << "' to file\n";
  ofstream of(path.str(), ios::binary);
  chunk->save_to_file(of);
}

Tile::Tile(ivec3 newpos, World* nworld): pos(newpos), world(nworld), chunksize(nworld->chunksize), loader(nworld->seed, newpos.x, newpos.y, newpos.z) {
	
	stringstream path;
	path << "saves/world/chunk" << pos.x << "x" << pos.y << "y" << pos.z << "z.dat";
	ifstream ifile(path.str(), ios::binary);
	if (ifile.good()) {
			cout << "loading '" << path.str() << "' from file\n";
			//chunks[pos] = parse_file(&ifile, pos.first, 0, pos.second, chunksize, nullptr);
			chunk = parse_file(ifile, pos.x, pos.y, pos.z, chunksize, nullptr);
	} else {
			cout << "generating '" << path.str() << "'\n";
			chunk = generate(pos);
			//return load_chunk(pos);
			//chunk = generate(pos);
	}
	//render_chunk_vectors(pos);
	//render(&world->glvecs);
}

void Tile::del(bool remove_faces) {
	chunk->del(remove_faces);
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
