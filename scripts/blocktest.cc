#include "blocks.h"
#include "materials.h"
#include "blockdata.h"
#include <bitset>

int main() {
	
	matstorage = new MaterialStorage();
	blocks = new BlockStorage();
	
	cout << "Block tests:" << endl;
	cout << "sizeof Block: " << sizeof(Block) << endl;
	cout << "sizeof Pixel: " << sizeof(Pixel) << endl;
	cout << "sizeof Chunk: " << sizeof(Chunk) << endl << endl;
	
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
	
	
	Pixel* pix = new Pixel(0,0,0, 1, 4, nullptr, nullptr);
	Block* block = pix->subdivide();
	block->get_chunk()->blocks[1][0][0]->get_pix()->subdivide();
	
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
		int gx, gy, gz;
		pix->global_position(&gx, &gy, &gz);
		cout << gx << ' ' << gy << ' ' << gz << ' ' << pix->scale << endl;
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
		int gx, gy, gz;
		pix->global_position(&gx, &gy, &gz);
		cout << gx << ' ' << gy << ' ' << gz << ' ' << pix->scale << endl;
	}
	
	cout << endl << "Built in iter_side (1,0,0)" << endl;
	for (const Pixel* pix : block->const_iter_side(ivec3(1,0,0))) {
		int gx, gy, gz;
		pix->global_position(&gx, &gy, &gz);
		cout << gx << ' ' << gy << ' ' << gz << ' ' << pix->scale << endl;
	}
	
	ifstream ifile("saves/Starting-World/chunks/0x0y-2z.dat", std::ios::binary);
	Block* bigblock = Block::from_file(ifile, 0, 0, 0, 64, nullptr, nullptr);
	
	int num_pix = 0;
	int start = clock();
	for (Pixel* pix : bigblock->iter()) {
		num_pix ++;
		for (int i = 0; i < 100000; i ++) {
			num_pix += i;
		}
	}
	int end = clock();
	cout << "num pix " << num_pix << endl;
	cout << "iter time: " << end - start << endl;
	
	
	cout << "all time : " << end - start << endl;
	
	cout << endl << "completed without errors" << endl;
	
	delete matstorage;
	delete blocks;
}
