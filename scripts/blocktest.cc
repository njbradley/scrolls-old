#include "blocks.h"
#include "materials.h"
#include "blockdata.h"

int main() {
	
	matstorage = new MaterialStorage();
	blocks = new BlockStorage();
	
	Pixel* pix = new Pixel(0,0,0, 1, 4, nullptr, nullptr);
	Block* block = pix->subdivide();
	block->get_chunk()->blocks[1][0][0]->get_pix()->subdivide();
	
	cout << endl << "Expected output (all): " << endl;
	block->all([] (Pixel* pix) {
		int gx, gy, gz;
		pix->global_position(&gx, &gy, &gz);
		cout << gx << ' ' << gy << ' ' << gz << ' ' << pix->scale << endl;
	});
	
	cout << endl << "Expected output (all_side 1 0 0): " << endl;
	block->all_side([] (Pixel* pix) {
		int gx, gy, gz;
		pix->global_position(&gx, &gy, &gz);
		cout << gx << ' ' << gy << ' ' << gz << ' ' << pix->scale << endl;
	}, 1, 0, 0);
	
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
	BlockIter blockiterside {block, ivec3(1,0,0), ivec3(1,1,1), [] (ivec3 pos, ivec3 startpos, ivec3 endpos) {
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
	for (Pixel* pix : block->iterside(ivec3(1,0,0))) {
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
	
	num_pix = 0;
	start = clock();
	bigblock->all([&] (Pixel* pix) {
		num_pix ++;
		for (int i = 0; i < 100000; i ++) {
			num_pix += i;
		}
	});
	end = clock();
	cout << "all time : " << end - start << endl;
	
	cout << endl << "completed without errors" << endl;
	
	delete matstorage;
	delete blocks;
}
