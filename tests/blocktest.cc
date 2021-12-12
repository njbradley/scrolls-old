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
	
	void iter_test() {
		
		Block* block = new Block();
		block->set_parent(nullptr, ivec3(0,0,0), 4);
		block->divide();
		
		block->set_child(ivec3(1,0,0), new Block(new Pixel(blocktypes::dirt.id)));
		block->set_child(ivec3(1,1,1), new Block(new Pixel(blocktypes::grass.id)));
		block->get(1,1,1)->subdivide();
		block->get(1,1,1)->set_child(ivec3(0,0,0), nullptr);
		
		out << "START " << endl;
		
		for (Pixel* pix : block->iter()) {
			out << pix << ' ' << pix->parbl->globalpos << ' ' << pix->parbl->scale << endl;
		}
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
		srand(123456);
		Block* block = gen_random_block(64);
		double start = getTime();
		int i = 0;
		for (Pixel* pix : block->iter()) {
			i ++;
		}
		double time = getTime() - start;
		out << "Total time: " << time << " over " << i << " blocks " << endl;
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
