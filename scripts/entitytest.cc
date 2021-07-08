#include "entity.h"
#include "cross-platform.h"

float t;

#define COLLIDETEST(box1, box2, expected) cout << box1 << " collide " << box2 << endl << '[' << expected << box1.collide(box2,0) << ']' << endl;
#define COLLIDETIMETEST(box1, box2, expected) box1.collide(box2, 999999999, &t); cout << box1 << " collide " << box2 << endl << '[' << expected << ' ' << t << ']' << endl;
#define CONTAINSTEST(box1, box2, expected) cout << box1 << " contains " << box2 << endl << '[' << expected << box1.contains(box2) << ']' << endl;

int main() {
	setup_backtrace();
	
	Hitbox box1 (vec3(20,20,20), vec3(-1,-1,-1), vec3(1,1,1));
	
	COLLIDETEST(box1, Hitbox(vec3(20,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(1,0,0))), true);
	COLLIDETEST(box1, Hitbox(vec3(22,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,1,0))), false);
	COLLIDETEST(box1, Hitbox(vec3(22,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(0,1,0))), true);
	COLLIDETEST(box1, Hitbox(vec3(22,20,18), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(1,0,0))), false);
	COLLIDETEST(box1, Hitbox(vec3(21,20,18), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,1,0))), false);
	COLLIDETEST(box1, Hitbox(vec3(21,20,18), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(-1.0f, vec3(0,1,0))), true);
	COLLIDETEST(box1, Hitbox(vec3(21,20,18), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(0,1,0))), true);
	COLLIDETEST(box1, Hitbox(vec3(22,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,1,0))), false);
	COLLIDETEST(box1, Hitbox(vec3(22,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,1,0))), false);
	COLLIDETEST(box1, Hitbox(vec3(22,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,1,0))), false);
	
	cout << endl;
	
	COLLIDETEST(
		Hitbox(vec3(1,0,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,0,1))),
		Hitbox(vec3(-1,0,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,0,1))), false
	);
	
	COLLIDETEST(
		Hitbox(vec3(1,0,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(0,0,1))),
		Hitbox(vec3(-1,0,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(-1.0f, vec3(0,0,1))), true
	);
	
	COLLIDETEST(
		Hitbox(vec3(1,0,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(2.0f, vec3(0,0,1))),
		Hitbox(vec3(-1,0,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(2.0f, vec3(0,0,1))), true
	);
	
	COLLIDETEST(
		Hitbox(vec3(1,1,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,0,1))),
		Hitbox(vec3(0,-1,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,0,1))), false
	);
	
	COLLIDETEST(
		Hitbox(vec3(1,1,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(0,0,1))),
		Hitbox(vec3(0,-1,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(0,0,1))), false
	);
	
	COLLIDETEST(
		Hitbox(vec3(1,1,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(-1.0f, vec3(0,0,1))),
		Hitbox(vec3(0,-1,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(-1.0f, vec3(0,0,1))), true
	);
	
	cout << endl;
	
	Hitbox fbox (vec3(1,1,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(-1.0f, vec3(0,0,1)));
	CONTAINSTEST(fbox.boundingbox(), fbox, true);
		
	cout << endl;
	
	COLLIDETIMETEST(
		Hitbox(vec3(0,0,0), vec3(-1,-1,-1), vec3(1,1,1)),
		Hitbox(vec3(3,0,0), vec3(-1,-1,-1), vec3(1,1,1), quat(1,0,0,0), vec3(-1,0,0)), 1
	);
	
	COLLIDETIMETEST(
		Hitbox(vec3(0,0,0), vec3(-1,-1,-1), vec3(1,1,1)),
		Hitbox(vec3(3,0,0), vec3(-1,-1,-1), vec3(1,1,1), quat(1,0,0,0), vec3(-2,0,0)), 0.5
	);
	
	COLLIDETIMETEST(
		Hitbox(vec3(0,0,0), vec3(-1,-1,-1), vec3(1,1,1)),
		Hitbox(vec3(-3,0,0), vec3(-1,-1,-1), vec3(1,1,1), quat(1,0,0,0), vec3(-1,0,0)), -1
	);
	
	COLLIDETIMETEST(
		Hitbox(vec3(0,0,0), vec3(-1,-1,-1), vec3(1,1,1)),
		Hitbox(vec3(-3,0,0), vec3(-1,-1,-1), vec3(1,1,1), quat(1,0,0,0), vec3(1,0,0)), 1
	);
	
	COLLIDETIMETEST(
		Hitbox(vec3(0,0,0), vec3(-1,-1,-1), vec3(1,1,1)),
		Hitbox(vec3(-5,0,0), vec3(-1,-1,-1), vec3(1,1,1), quat(1,0,0,0), vec3(0.5,0,0)), 6
	);
	
	COLLIDETIMETEST(
		Hitbox(vec3(0,0,0), vec3(-1,-1,-1), vec3(1,1,1)),
		Hitbox(vec3(-5,0,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(1,0,0)), vec3(1,0,0)), 3
	);
	
	COLLIDETIMETEST(
		Hitbox(vec3(0,0,0), vec3(-1,-1,-1), vec3(1,1,1)),
		Hitbox(vec3(-5,0,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(0,1,0)), vec3(1,0,0)), 2.618
	);
	
	COLLIDETIMETEST(
		Hitbox(vec3(0,0,0), vec3(-1,-1,-1), vec3(1,1,1)),
		Hitbox(vec3(-5,0,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(0,0,1)), vec3(1,0,0)), 2.618
	);
	
	
	return 0;
}
