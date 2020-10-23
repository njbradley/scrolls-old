#include "classes.h"

#include <time.h>

#include "entity.h"
#include "menu.h"
#include "world.h"
#include "blockdata.h"
#include "tiles.h"
#include "blocks.h"
#include "items.h"
#include "crafting.h"
#include "terrain.h"
#include "materials.h"


float initialFoV = 110.0f;

void set_display_env(vec3 new_clear_color, int new_view_dist) {
	
}

void crash(long long err_code) {
	cout << "fatal error encountered! shutting down cleanly: err code: " << err_code << endl;
}

void hard_crash(long long err_code) {
	cout << "fatal error encountered! unable to close cleanly: err code: " << err_code << endl;
}

int main() {
	
	matstorage = new MaterialStorage();
	blocks = new BlockStorage();
	itemstorage = new ItemStorage();
	recipestorage = new RecipeStorage();
	entitystorage = new EntityStorage();
	
	World world("name", nullptr, 12345);
	
	clock_t start = clock();
	
	for (int i = 0; i < 64; i ++) {
		for (int j = 0; j < 64; j ++) {
			for (int k = 0; k < 64; k ++) {
				world.loader.gen_func(ivec3(i,j+128,k));
			}
		}
	}
	
	clock_t total = clock() - start;
	cout << "pure gen func: " << double(total)/CLOCKS_PER_SEC << endl;
	
	start = clock();
	
	Tile tile(ivec3(0,2,0), &world);
	
	total = clock() - start;
	cout << "tile gen func: " << double(total)/CLOCKS_PER_SEC << endl;
	
}
