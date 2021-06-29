#include "entity.h"
#include "cross-platform.h"

#define COLLIDETEST(box1, box2, expected) cout << box1 << " collide " << box2 << ' ' << expected << box1.collide(box2) << endl;
#define CONTAINSTEST(box1, box2, expected) cout << box1 << " contains " << box2 << ' ' << expected << box1.contains(box2) << endl;

int main() {
	setup_backtrace();
	
	Hitbox box1 (vec3(20,20,20), vec3(-1,-1,-1), vec3(1,1,1));
	
	COLLIDETEST(box1, FreeHitbox(vec3(20,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(1,0,0))), true);
	COLLIDETEST(box1, FreeHitbox(vec3(22,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,1,0))), false);
	COLLIDETEST(box1, FreeHitbox(vec3(22,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(0,1,0))), true);
	COLLIDETEST(box1, FreeHitbox(vec3(22,20,18), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(1,0,0))), false);
	COLLIDETEST(box1, FreeHitbox(vec3(21,20,18), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,1,0))), false);
	COLLIDETEST(box1, FreeHitbox(vec3(21,20,18), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(-1.0f, vec3(0,1,0))), true);
	COLLIDETEST(box1, FreeHitbox(vec3(21,20,18), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(0,1,0))), true);
	COLLIDETEST(box1, FreeHitbox(vec3(22,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,1,0))), false);
	COLLIDETEST(box1, FreeHitbox(vec3(22,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,1,0))), false);
	COLLIDETEST(box1, FreeHitbox(vec3(22,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,1,0))), false);
	
	cout << endl;
	
	COLLIDETEST(
		FreeHitbox(vec3(1,0,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,0,1))),
		FreeHitbox(vec3(-1,0,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,0,1))), false
	);
	
	COLLIDETEST(
		FreeHitbox(vec3(1,0,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(0,0,1))),
		FreeHitbox(vec3(-1,0,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(-1.0f, vec3(0,0,1))), true
	);
	
	COLLIDETEST(
		FreeHitbox(vec3(1,0,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(2.0f, vec3(0,0,1))),
		FreeHitbox(vec3(-1,0,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(2.0f, vec3(0,0,1))), true
	);
	
	COLLIDETEST(
		FreeHitbox(vec3(1,1,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,0,1))),
		FreeHitbox(vec3(0,-1,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,0,1))), false
	);
	
	COLLIDETEST(
		FreeHitbox(vec3(1,1,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(0,0,1))),
		FreeHitbox(vec3(0,-1,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(0,0,1))), false
	);
	
	COLLIDETEST(
		FreeHitbox(vec3(1,1,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(-1.0f, vec3(0,0,1))),
		FreeHitbox(vec3(0,-1,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(-1.0f, vec3(0,0,1))), true
	);
	
	cout << endl;
	
	FreeHitbox fbox (vec3(1,1,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(-1.0f, vec3(0,0,1)));
	CONTAINSTEST(fbox.boundingbox(), fbox, true);
		
	
	return 0;
}
