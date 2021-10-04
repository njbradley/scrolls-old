#include "scrolls/tests.h"

#include "scrolls/entity.h"
#include "scrolls/blocks.h"

struct EntityTest : Test {
	static constexpr vec3 ones = vec3(1,1,1);
	static constexpr vec3 zeros = vec3(0,0,0);
	
	EntityTest() {
		name = "entitytest";
	}
	
	void collide_tests() {
		Hitbox centerbox (zeros, -ones, ones);
		
		TASSERT(centerbox.collide(Hitbox(-ones, -ones, ones)));
		TASSERT(centerbox.collide(Hitbox(ones, -ones, ones)));
		TASSERT_NOT(centerbox.collide(Hitbox(vec3(2,0,0), -ones, ones)));
		TASSERT_NOT(centerbox.collide(Hitbox(vec3(50,50,50), -ones, ones)));
		
		
	}
	
	void vel_collide_tests() {
		Hitbox centerbox (zeros, -ones, ones);
		
		// TASSERT_NOT(centerbox.collide(Hitbox(vec3(2,0,0), -ones, ones, quat(1,0,0,0), vec3(0,0,0)), 0));
		// TASSERT(centerbox.collide(Hitbox(vec3(2,0,0), -ones, ones, quat(1,0,0,0), vec3(-1,0,0)), 1));
		
		// TASSERT_NOT(centerbox.collide(Hitbox(vec3(2,0,0), -ones, ones, quat(1,0,0,0), vec3(1,0,0)), 1));
	}
	
	void line_tests() {
		Line xax (vec3(0,0,0), vec3(1,0,0));
		Line yax (vec3(0,0,0), vec3(0,1,0));
		
		TASSERT_EQ(vec3(0,0,0), xax.collision(yax));
		TASSERT_EQ(vec3(1,0,0), xax.collision(Line(vec3(1,1,0), vec3(0,-1,0))));
	}
	
	void transform_tests() {
		Hitbox box1 (zeros, -ones, ones, glm::angleAxis(3.14f/2, vec3(0,0,1)));
		
		TASSERT_EQ(vec3(0,0,0), box1.transform_out(box1.transform_in(vec3(0,0,0))));
		TASSERT_EQ(vec3(0,0,0), box1.transform_in(box1.transform_out(vec3(0,0,0))));
		
		TASSERT_EQ(vec3(0,1000,0), box1.transform_out(box1.transform_in(vec3(0,1000,0))));
		TASSERT_EQ(vec3(0,1000,0), box1.transform_in(box1.transform_out(vec3(0,1000,0))));
		
		TASSERT_EQ(vec3(0,10,0), box1.transform_out(box1.transform_in(vec3(0,10,0))));
		TASSERT_EQ(vec3(0,10,0), box1.transform_in(box1.transform_out(vec3(0,10,0))));
	}
	
	void test() {
		// line_tests();
		
		collide_tests();
		vel_collide_tests();
	}
};

EXPORT_PLUGIN(EntityTest);
