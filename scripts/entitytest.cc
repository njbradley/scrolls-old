#include "entity.h"
#include "cross-platform.h"

#define COLLIDETEST(box1, box2) cout << box1 << " collide " << box2 << ' ' << box1.collide(box2) << endl;
#define CONTAINSTEST(box1, box2) cout << box1 << " contains " << box2 << ' ' << box1.contains(box2) << endl;

int main() {
	setup_backtrace();
	
	Hitbox box1 (vec3(20,20,20), vec3(-1,-1,-1), vec3(1,1,1));
	
	cout << box1.collide(Hitbox(vec3(21,21,21), vec3(-1,-1,-1), vec3(1,1,1))) << endl;
	cout << box1.collide(Hitbox(vec3(22,22,22), vec3(-1,-1,-1), vec3(1,1,1))) << endl;
	
	cout << box1.collide(FreeHitbox(box1, glm::angleAxis(3.14f/2, vec3(1,0,0)))) << endl;
	cout << box1.collide(FreeHitbox(vec3(20,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(3.14f/2, vec3(1,0,0)))) << endl;
	cout << box1.collide(FreeHitbox(vec3(22,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,1,0)))) << endl;
	cout << box1.collide(FreeHitbox(vec3(22,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(0,1,0)))) << endl;
	cout << box1.collide(FreeHitbox(vec3(22,20,18), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(1,0,0)))) << endl;
	cout << box1.collide(FreeHitbox(vec3(22,20,18), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,1,0)))) << endl;
	cout << box1.collide(FreeHitbox(vec3(22,20,18), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(-1.0f, vec3(0,1,0)))) << endl;
	cout << box1.collide(FreeHitbox(vec3(22,20,18), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(0,1,0)))) << endl;
	
	COLLIDETEST(box1, FreeHitbox(vec3(20,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(1,0,0))));
	COLLIDETEST(box1, FreeHitbox(vec3(22,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,1,0))));
	COLLIDETEST(box1, FreeHitbox(vec3(22,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(0,1,0))));
	COLLIDETEST(box1, FreeHitbox(vec3(22,20,18), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(1,0,0))));
	COLLIDETEST(box1, FreeHitbox(vec3(21,20,18), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,1,0))));
	COLLIDETEST(box1, FreeHitbox(vec3(21,20,18), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(-1.0f, vec3(0,1,0))));
	COLLIDETEST(box1, FreeHitbox(vec3(21,20,18), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(0,1,0))));
	COLLIDETEST(box1, FreeHitbox(vec3(22,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,1,0))));
	COLLIDETEST(box1, FreeHitbox(vec3(22,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,1,0))));
	COLLIDETEST(box1, FreeHitbox(vec3(22,20,20), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(0.0f, vec3(0,1,0))));
	
	return 0;
}
