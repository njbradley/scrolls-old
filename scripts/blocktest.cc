#include "blocks.h"
#include "materials.h"
#include "blockdata.h"
#include "cross-platform.h"
#include <bitset>

int main() {
	setup_backtrace();
	
	matstorage = new MaterialStorage();
	blocks = new BlockStorage();
	
	cout << "Block tests:" << endl;
	cout << "sizeof Block: " << sizeof(Block) << endl;
	cout << "sizeof Pixel: " << sizeof(Pixel) << endl;
	
	cout << "File tests: " << endl;
	stringstream datastream;
	Block::write_pix_val(datastream, 0b00, 22);
	Block::write_pix_val(datastream, 0b01, 5);
	Block::write_pix_val(datastream, 0b11, 283);
	while (!datastream.eof()) {
		cout << std::bitset<8>(datastream.get()) << endl;
	}
	cout << endl;
	
	stringstream data2;
	
	Block::write_pix_val(data2, 0b00, -5);
	Block::write_pix_val(data2, 0b01, 524234);
	Block::write_pix_val(data2, 0b11, 99999);
	for (int i = 0; i < 3; i ++) {
		char type;
		unsigned int value;
		Block::read_pix_val(data2, &type, &value);
		cout << std::bitset<2>(type) << ' ' << value << ' ' << std::bitset<32>(value) << endl;
	}
	cout << endl;
	
	cout << "making blocks" << endl;
	
	BlockContainer container(4);
	container.block->set_pixel(new Pixel(1));
	container.block->subdivide();
	container.block->get(1,0,0)->subdivide();
	
	Block* block = container.block;
	
	cout << endl << "New output (alliter):" << endl;
	BlockIter blockiter {block, ivec3(0,0,0), ivec3(1,1,1), [] (ivec3 pos, ivec3 startpos, ivec3 endpos) {
		pos.z++;
		if (pos.z > endpos.z) {
			pos.z = startpos.z;
			pos.y ++;
			if (pos.y > endpos.y) {
				pos.y = startpos.y;
				pos.x ++;
			}
		}
		return pos;
	}};
	for (Pixel* pix : block->iter()) {
		cout << pix->parbl->globalpos << ' ' << pix->parbl->scale << endl;
	}
	
	cout << endl << "New output (alliter from 1,0,0 to 1,1,1):" << endl;
	BlockIter blockiter_side {block, ivec3(1,0,0), ivec3(1,1,1), [] (ivec3 pos, ivec3 startpos, ivec3 endpos) {
		pos.z++;
		if (pos.z > endpos.z) {
			pos.z = startpos.z;
			pos.y ++;
			if (pos.y > endpos.y) {
				pos.y = startpos.y;
				pos.x ++;
			}
		}
		return pos;
	}};
	for (Pixel* pix : block->iter_side(ivec3(1,0,0))) {
		cout << pix->parbl->globalpos << ' ' << pix->parbl->scale << endl;
	}
	
	cout << endl << "Built in iter_side (1,0,0)" << endl;
	for (const Pixel* pix : block->const_iter_side(ivec3(1,0,0))) {
		cout << pix->parbl->globalpos << ' ' << pix->parbl->scale << endl;
	}
	
	cout << endl << "Test empty iter object" << endl;
	BlockIter emptyiter {.base = nullptr};
	for (Pixel* pix : emptyiter) {
		cout << "BBBADAADAD" << endl;
	}
	
	ivec3 chosen_block(1,1,1);
	Block* subblock = container.get_global(chosen_block.x, chosen_block.y, chosen_block.z,1);
	
	cout << endl << "Example touching " << endl;
	
	const ivec3 dir_array[6] =    {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
	for (ivec3 unit_dir : dir_array) {
		ivec3 dir = unit_dir * subblock->scale;
		Block* tmpblock = container.get_global(chosen_block.x+dir.x, chosen_block.y+dir.y, chosen_block.z+dir.z, subblock->scale);
		if (tmpblock != nullptr) {
			for (Pixel* pix : tmpblock->iter_side(-unit_dir)) {
				cout << pix->parbl->globalpos << ' ' << pix->parbl->scale << endl;
			}
		}
	}
	
	cout << endl << "Touching iter test "<< endl;
	
	for (Pixel* pix : subblock->iter_touching()) {
		cout << pix->parbl->globalpos << ' ' << pix->parbl->scale << endl;
	}
	
	cout << endl << "Group iter test " << endl;
	
	Pixel* startpix = block->get_global(0,0,0,1)->pixel;
	for (Pixel* pix : startpix->iter_group(&container)) {
		cout << pix->parbl->globalpos << ' ' << pix->parbl->scale << endl;
	}
	
	cout << endl << "Get global tests" << endl;
	
	ivec4 checks[] {
		{0, 0, 0, 1},
		{3, 3, 3, 1},
		{0, 0, 0, 2},
		{3, 3, 3, 4},
	};
	
	for (ivec4 pos : checks) {
		Block* b = block->get_global(pos.x, pos.y, pos.z, pos.w);
		cout << b->globalpos << ' ' << b->scale << endl;
	}
	
	cout << "local" << endl;
	
	for (ivec4 pos : checks) {
		Block* b = block->get_global(0,0,0,1)->get_global(pos.x, pos.y, pos.z, pos.w);
		cout << b->globalpos << ' ' << b->scale << endl;
	}
	
	cout << endl;
	
	ifstream ifile("saves/Starting-World/chunks/-1x1y-2z.dat", std::ios::binary);
	BlockContainer bigcontainer(64);
	bigcontainer.block->from_file(ifile);
	Block* bigblock = bigcontainer.block;
	
	cout << endl << "get global speed test" << endl;
	
	int start = clock();
	for (int i = 0; i < 1000000; i ++) {
		Block* block = bigblock->get_global(rand()%64, rand()%64, rand()%64, 1);
	}
	cout << double(clock() - start) / CLOCKS_PER_SEC << " result" << endl;
	
	cout << "next to get_globals" << endl;
	
	start = clock();
	Block* chosen = bigblock->get_global(32, 32, 32, 4);
	ivec3 dir = dir_array[rand()%6];
	cout << chosen << endl;
	for (int i = 0; i < 1000000; i ++) {
		Block* block = chosen->get_global(ivec3(32,32,32) + dir * 4, 1);
	}
	cout << double(clock() - start) / CLOCKS_PER_SEC << " result" << endl;
	
	int num_pix = 0;
	start = clock();
	for (Pixel* pix : bigblock->iter()) {
		num_pix ++;
	}
	int end = clock();
	cout << endl << "num pix " << num_pix << endl;
	cout << "iter time: " << end - start << endl;
	
	cout << endl << "Big group iter: " << endl;
	num_pix = 0;
	startpix = bigblock->get_global(0, 63, 0, 1)->pixel;
	start = clock();
	for (Pixel* pix : startpix->iter_group(&bigcontainer)) {
		num_pix ++;
	}
	end = clock();
	cout << "Num pix in group : " << num_pix << endl << endl;
	cout << "Time : " << end - start << endl << endl;
	
	cout << endl << "Custom test "<< endl;
	startpix = bigblock->get_global(0, 63, 0, 1)->pixel;
	start = clock();
	BlockGroupIter iter = startpix->iter_group(&bigcontainer);
	// for (vector<Pixel*>::iterator i = iter.begin(); i != iter.end(); i ++) {
	// 	num_pix ++;
	// }
	end = clock();
	cout << "other time : " << end - start << endl;
	
	cout << "mem overhead " << sizeof(vector<Pixel*>) << ' ' << sizeof(unordered_set<Pixel*>) << endl;
	
	start = clock();
	for (int i = 0; i < 1000000; i ++) {
		unordered_set<Pixel*> pset;
		pset.emplace(nullptr);
		pset.emplace(startpix);
	}
	end = clock();
	cout << "computation overhead : set " << end - start << endl;
	
	start = clock();
	for (int i = 0; i < 1000000; i ++) {
		vector<Pixel*> pset;
		pset.push_back(nullptr);
		pset.push_back(startpix);
	}
	end = clock();
	cout << "computation overhead : vector " << end - start << endl;
	
	
	
	cout << endl << "completed without errors" << endl;
	
	delete matstorage;
	delete blocks;
}
