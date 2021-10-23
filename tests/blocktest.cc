#include "scrolls/tests.h"

#include "scrolls/entity.h"
#include "scrolls/blocks.h"

struct BlockTest : Test { public:
	
	BlockTest() {
		name = "blocktest";
	}
	
	void test() {
		
		unsigned int val = 2374982379;
		unsigned int result;
		char type;
		
		std::stringstream stream;
		
		Block::write_pix_val(stream, 0b00, val);
		Block::read_pix_val(stream, &type, &result);
		
		cout << std::hex << val << ' ' << result << std::dec << endl;
		
		TASSERT_EQ(val, result);
	}
};

EXPORT_PLUGIN(BlockTest);
