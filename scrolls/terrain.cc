#include "terrain.h"
#include "world.h"
#include "blockdata.h"
#include "blocks.h"


DEFINE_PLUGIN(TerrainLoader);
EXPORT_PLUGIN(TerrainLoader);


TerrainLoader::TerrainLoader(int nseed): seed(nseed) {
	
}

Block* TerrainLoader::generate_chunk(ivec3 pos) {
	pos *= World::chunksize;
	stringstream ss;
	gen_block(ss, pos.x, pos.y, pos.z, World::chunksize);
	return new Block(ss);
}

Blocktype TerrainLoader::gen_block(ostream& ofile, int gx, int gy, int gz, int scale) {
  // Blocktype val = gen_func(ivec4(gx, gy, gz, scale));
  // cout << int(val) << ' ' << ivec3(gx,gy,gz) << ' ' << scale << endl;
  if (scale == 1) {// or val != -1) {
		Blocktype val = gen_func(ivec3(gx, gy, gz));
    Block::write_pix_val(ofile, 0b00, val);
    return val;
  } else {
    stringstream ss;
    ss << char(0b11000000);
    Blocktype val = gen_block(ss, gx, gy, gz, scale/csize);
    bool all_same = val != -1;
    for (int x = 0; x < csize; x ++) {
      for (int y = 0; y < csize; y ++) {
        for (int z = 0; z < csize; z ++) {
          if (x > 0 or y > 0 or z > 0) {
            Blocktype newval = gen_block(ss, gx+x*(scale/csize), gy+y*(scale/csize), gz+z*(scale/csize), scale/csize);
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

Blocktype TerrainLoader::gen_func(ivec3 pos) {
	if (pos.y < pos.x) {
		// cout << blocktypes::dirt.id << endl;
		return blocktypes::dirt.id;
	}
	return 0;
}
