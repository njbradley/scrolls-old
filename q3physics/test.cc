#include <iostream>
#include "q3.h"

using std::cout;
using std::endl;

/*
int main() {
	q3BodyDef bodyDef;
	q3Body* floor = scene.CreateBody(bodyDef);
	
	q3BoxDef boxDef; // See q3Box.h for settings details
	q3Transform localSpace; // Contains position and orientation, see q3Transform.h for details
	q3Identity(localSpace); // Specify the origin, and identity orientation
	
	// Create a box at the origin with width, height, depth = (1.0, 1.0, 1.0)
	// and add it to a rigid body. The transform is defined relative to the owning body
	boxDef.Set(localSpace, q3Vec3(100,4,100));
	floor->AddBox(boxDef);
	
	bodyDef.bodyType = eDynamicBody;
	bodyDef.position.y = 10;
	q3Body* body = scene.CreateBody(bodyDef);
	
	boxDef.Set(localSpace, q3Vec3(1,1,1));
	body->AddBox(boxDef);
	
	for (int i = 0; i < 100; i ++) {
		cout << body->GetTransform().position.y << endl;
		scene.Step();
	}
	
}
*/
