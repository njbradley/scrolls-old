#ifndef TERRAIN
#define TERRAIN

#include "terrain-predef.h"
#include "blockdata-predef.h"
#include "generative.h"


Island::Island(ChunkLoader* chunk, int nseed, int x, int y, int nwidth, int nheight): TerrainBase(chunk, nseed), xpos(x), ypos(y), width(nwidth), height(nheight) {
	generate_arrays();
}

pair<int,int> Island::generate_bounds(int x, int y) {
	set_seed(seed);
  int ix = x-128;
  int iy = y-128;
  int top = get_heightmap(x, y)-(ix*ix+iy*iy)*0.004+height;
  int bottom = (scaled_perlin(x,y,32,80) + scaled_perlin(x,y,16,20) + scaled_perlin(x,y,4,5));// + (ix*ix+iy*iy)*0.01;
  bottom = bottom * bottom/20;
  bottom = -bottom + (ix*ix+iy*iy)*0.01 + 120;
  return pair<int,int>(top, bottom);
}

int Island::get_height(int x, int y) {
	return height_array[x][y];
}

void Island::generate_arrays() {
	for (int x = 0; x < chunksize; x ++) {
		for (int y = 0; y < chunksize; y ++) {
			pair<int,int> bounds = generate_bounds(x, y);
			if (bounds.first > bounds.second) {
				height_array[x][y] = bounds.first;
				base_array[x][y] = bounds.second;
			} else {
				height_array[x][y] = -1;
				base_array[x][y] = -1;
			}
		}
	}
}

char Island::gen_func(int x, int y, int z) {
	double height = height_array[x][z];
	double base = base_array[x][z];
	
	if (y > base and y < height-5) {
			return blocks->names["rock"];
	} if (y < height-1 and y > base) {
			return blocks->names["dirt"];
	} if (y < height and y > base) {
			return blocks->names["grass"];
	}
	return -1;
}

int Island::priority() {
	return 1;
}






Trees::Trees(ChunkLoader* chunk, int nseed, string ntype): TerrainObject(chunk,nseed), type(ntype) {
	tree_data = new int[num_trees][20][20][20];
	//memset(tree_data, -1, num_trees*20*20*20);
	for (int i = 0; i < num_trees; i ++) {
		for (int j = 0; j < 20; j ++) {
			for (int k = 0; k < 20; k ++) {
				for (int l = 0; l < 20; l ++) {
					tree_data[i][j][k][l] = -1;
				}
			}
		}
		int x,y;
		do {
			x = rand()%255;
			y = rand()%255;
		} while (parent->get_height(x, y) == -1);
		positions.push_back(pair<int,int>(x,y));
		generate_tree(i);
	}
}

void Trees::generate_tree(int index) {
	pair<int,int> pos = positions[index];
	memset(tree_data[index], -1, 20*20*20);
	int num_branches = 8+rand()%5;
	for (int i = 0; i < num_branches; i ++) {
		double x = 10 + rand()%2;
		double y = 0;
		double z = 10 + rand()%2;
		int dx = rand()%11-5;
		int dz = rand()%11-5;
		int height = 10+rand()%10;
		for (int j = 0; j < height; j ++) {
			x += (dx/(double(height-j)*2));
			y ++;
			z += (dz/(double(height-j)*2));
			if (x < 20 and x >= 0 and y < 20 and y >= 0 and z < 20 and z >= 0) {
				tree_data[index][int(x)][int(y)][int(z)] = blocks->names["bark"];
				tree_data[index][int(x)+1][int(y)][int(z)] = blocks->names["bark"];
				tree_data[index][int(x)][int(y)][int(z)+1] = blocks->names["bark"];
				tree_data[index][int(x)+1][int(y)][int(z)+1] = blocks->names["bark"];
			}
		}
	}
	
	int leaves_max = 8 + rand()%8;
	for (int leaves_height = 0; leaves_height < leaves_max; leaves_height ++) {
		for (int x = 1; x < 20; x ++) {
			for (int y = 1; y < 20; y ++) {
				for (int z = 1; z < 20; z ++) {
					if (tree_data[index][x][y][z] == -1 and tree_data[index][x][y-1][z] == blocks->names["bark"] or tree_data[index][x][y-1][z] == blocks->names["leaves"]) {
						int score = 0;
						for (int ox = -1; ox < 2; ox ++) {
							for (int oz = -1; oz < 2; oz ++) {
								if (tree_data[index][x+ox][y-1][z+oz] == blocks->names["bark"] or tree_data[index][x+ox][y-1][z+oz] == blocks->names["leaves"]) {
									score ++;
								}
							}
						}
						//cout << leaves_height-(leaves_max/2) << ' ' << score/5 << endl;
						if (score/5+rand()%3-1 > leaves_height-(leaves_max/2)) {
							tree_data[index][x][y][z] = blocks->names["leaves"];
							for (int ox = -1; ox < 2; ox ++) {
								for (int oz = -1; oz < 2; oz ++) {
									if (rand()%(leaves_max/2) < -(leaves_height-leaves_max/2) and tree_data[index][x+ox][y-1][z+oz] != blocks->names["bark"]) {
										tree_data[index][x+ox][y][z+oz] = blocks->names["leaves"];
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

char Trees::gen_func(int x, int y, int z) {
	for (int i = 0; i < num_trees; i ++) {
		pair<int,int> pos = positions[i];
		int height = parent->get_height(pos.first,pos.second);
		if (x < pos.first+10 and x >= pos.first-10 and z < pos.second+10 and z >= pos.second-10 and y < height + 20 and  y >= height ) {
			char val = tree_data[i][x-pos.first+10][y-height][ z-pos.second+10];
			if (val != -1) {
				return val;
			}
		}
	}
	return -1;
}

int Trees::priority() {
	return 2;
}



Path::Path(ChunkLoader* chunk, int nseed): TerrainObject(chunk, nseed) {
	int num_points = 3+rand()%7;
	int x,z;
	do {
		x = rand()%255;
		z = rand()%255;
		height = parent->get_height(x, z);
	} while (height == -1);
	for (int i = 0; i < num_points; i ++) {
		path_points.push_back(pair<int,int>(x,z));
		if (rand()%2 == 0) {
			x += (rand()%2*2-1)*10;
		} else {
			z += (rand()%2*2-1)*10;
		}
		cout << x << ' ' << z << endl;
	}
	cout << height << endl;
}

char Path::gen_func(int x, int y, int z) {
	bool last_in_x = false;
	bool last_in_z = false;
	for (pair<int,int> pos : path_points) {
		int h = parent->get_height(pos.first, pos.second);
		bool in_x = x >= pos.first-3 and x < pos.first+3;
		bool in_z = z >= pos.second-3 and z < pos.second+3;
		if (y >= height-1 and  y < height+1) {
			if (in_x and in_z) {
				return blocks->names["stone"];
			} else if ((in_x and last_in_z) or (last_in_x and in_z)) {
				return blocks->names["rock"];
			}
		} else if (y >= height+1 and y < height+8 and in_x and in_z) {
			return 0;
		} else if ( y > h and y < height and in_x and in_z) {
			return blocks->names["rock"];
		}
		last_in_x = in_x;
		last_in_z = in_z;
	}
	return -1;
}
			
int Path::priority() {
	return 3;
}




ChunkLoader::ChunkLoader(int nseed, int x, int y): seed(nseed), xpos(x), ypos(y) {
	terrain.push_back(new Island(this, seed, 0, 0, 100, 100));
	objs.push_back(new Path(this, seed));
	//objs.push_back(new Trees(this, seed, "oak"));
}

int ChunkLoader::get_height(int x, int y) {
	int height = -1;
	for (TerrainBase* base : terrain) {
		if (base->get_height(x,y)) {
			height = base->get_height(x,y);
		}
	}
	return height;
}

char ChunkLoader::gen_func(int x, int y, int z) {
	char val = 0;
	int priority = 0;
	for (TerrainBase* base : terrain) {
		char newval = base->gen_func(x,y,z);
		if (newval != -1 and base->priority() > priority) {
			val = newval;
			priority = base->priority();
		}
		if (val > 15) {
			cout << 159 << endl;
		}
	}
	for (TerrainObject* obj : objs) {
		char newval = obj->gen_func(x,y,z);
		if (newval != -1 and obj->priority() > priority) {
			val = newval;
			priority = obj->priority();
		}
		if (val > 15) {
			cout << "169" << endl;
		}
	}
	return val;
}


#endif
