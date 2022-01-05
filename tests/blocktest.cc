#include "scrolls/tests.h"

#include "scrolls/entity.h"
#include "scrolls/blocks.h"
#include "scrolls/blockiter.h"
#include "scrolls/blockdata.h"
#include "scrolls/fileformat.h"

struct BlockTest : Test { public:
	PLUGIN_HEAD(BlockTest);
	
	BlockTest() {
		name = "blocktest";
	}
	
	void iter_verify_block(Block* block, BlockIterator<Block>& iter) {
		if (block == nullptr) return;
		TASSERT_EQ(block, *iter);
		++iter;
		if (block->continues) {
			for (int i = 0; i < 8; i ++) {
				iter_verify_block(block->children[i], iter);
			}
		}
	}
	
	void iter_verify_pixel(Block* block, PixelIterator<Block>& iter) {
		if (block == nullptr) return;
		if (block->continues) {
			for (int i = 0; i < 8; i ++) {
				iter_verify_pixel(block->children[i], iter);
			}
		} else {
			TASSERT_EQ(block->pixel, *iter);
			++iter;
		}
	}
	
	void iter_verify_pixel_dir(Block* block, DirPixelIterator<Block>& iter) {
		if (block == nullptr) return;
		cout << block << ' ' << block->globalpos << ' ' << block->scale << endl;
		if (block->continues) {
			for (int i = 0; i < 4; i ++) {
				iter_verify_pixel_dir(block->children[i], iter);
			}
		} else {
			TASSERT_EQ(block->pixel, *iter);
			++iter;
		}
	}
	
	void iter_test() {
		
		Block* block = new Block();
		block->set_parent(nullptr, ivec3(0,0,0), 4);
		block->divide();
		
		block->set_child(ivec3(1,0,0), new Block(new Pixel(blocktypes::dirt.id)));
		block->set_child(ivec3(1,1,1), new Block(new Pixel(blocktypes::grass.id)));
		block->get(1,1,1)->subdivide();
		block->get(1,1,1)->set_child(ivec3(0,0,0), nullptr);
		
		
		BlockIterable<BlockIterator<Block>> blockiter (block);
		BlockIterator<Block> biter = blockiter.begin();
		iter_verify_block(block, biter);
		
		
		BlockIterable<PixelIterator<Block>> pixeliter (block);
		PixelIterator<Block> piter = pixeliter.begin();
		iter_verify_pixel(block, piter);
		
		BlockIterable<DirPixelIterator<Block>> dirpixeliter (block, ivec3(-1,0,0));
		DirPixelIterator<Block> dpiter = dirpixeliter.begin();
		iter_verify_pixel_dir(block, dpiter);
		
		
		out << "START " << endl;
		
		for (Pixel* pix : block->iter()) {
			out << pix << ' ' << pix->parbl->globalpos << ' ' << pix->parbl->scale << endl;
		}
		
		out << "END" << endl;
		
		Hitbox hitbox (vec3(2), vec3(-0.5), vec3(0.5));
		// cout << hitbox << " start " << endl;
		//
		// FreeBlockIter freeiter (block, hitbox);
		// for (Pixel* pix : freeiter) {
		// 	cout << pix << endl;
		// 	cout << pix->parbl->hitbox().contains(hitbox) << endl;
		// }
		//
		// cout << endl;
		
		HitboxIterable<FreePixelIterator<Block>> freeiter2 (block, hitbox);
		for (Pixel* pix : freeiter2) {
			cout << pix << endl;
			cout << pix->parbl->hitbox().contains(hitbox) << endl;
		}
		
		cout << "done" << endl;
	}
	
	Block* gen_random_block(int scale) {
		if (scale < 2 or rand()%4 == 0) {
			return new Block(new Pixel(0));
		} else {
			Block* block = new Block();
			block->divide();
			for (int i = 0; i < csize3; i ++) {
				if (rand()%3 != 0) {
					block->set_child(block->posof(i), gen_random_block(scale/2));
				}
			}
			return block;
		}
	}
	
	void speed_iter_test() {
		srand(12345);
		Block* block = gen_random_block(64);
		Hitbox box (vec3(32,32,32), vec3(-1,-1,-1), vec3(1,1,1));
		double start = getTime();
		int i = 0;
		for (Pixel* pix : block->iter()) {
			i ++;
		}
		double time = getTime() - start;
		out << "Total time: " << time << " over " << i << " blocks " << endl;
		
		// start = getTime();
		// i = 0;
		// for (Pixel* pix : FullFreePixelIterable<Block>(block, box)) {
		// 	i ++;
		// }
		// time = getTime() - start;
		// out << "Total free time: " << time << endl;
		
		
		start = getTime();
		i = 0;
		for (Pixel* pix : HitboxIterable<FreePixelIterator<Block>>(block, box)) {
			i ++;
		}
		time = getTime() - start;
		out << "Total free time: " << time << endl;
		
	}
	
	void test() {
		
		uint64 val = 2374982379;
		uint64 result;
		char type;
		
		std::stringstream stream;
		
		FileFormat::write_variable(stream, val<<2);
		result = FileFormat::read_variable(stream);
		type = result % 4;
		result /= 4;
		
		out << std::hex << val << ' ' << result << std::dec << endl;
		
		TASSERT_EQ(val, result);
		
		iter_test();
		speed_iter_test();
	}
};

EXPORT_PLUGIN(BlockTest);
