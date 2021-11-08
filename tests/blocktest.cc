#include "scrolls/tests.h"

#include "scrolls/entity.h"
#include "scrolls/blocks.h"
#include "scrolls/blockiter.h"
#include "scrolls/blockdata.h"

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
	
	void test() {
		
		unsigned int val = 2374982379;
		unsigned int result;
		char type;
		
		std::stringstream stream;
		
		Block::write_pix_val(stream, 0b00, val);
		Block::read_pix_val(stream, &type, &result);
		
		out << std::hex << val << ' ' << result << std::dec << endl;
		
		TASSERT_EQ(val, result);
		
		iter_test();
	}
};

EXPORT_PLUGIN(BlockTest);
