#ifndef Q3PH_ENTITY
#define Q3PH_ENTITY

#include "scrolls/classes.h"
#include "scrolls/entity.h"

#include "q3.h"

vec3 q3toglm(q3Vec3 pos);
quat q3toglm(q3Quaternion rot);
glm::mat3 q3toglm(q3Mat3 mat);

q3Vec3 glmtoq3(vec3 pos);
q3Quaternion glmtoq3(quat rot);
q3Mat3 glmtoq3(glm::mat3 mat);

class Q3Wrapper { public:
	Movingbox* movingbox;
	q3Body* q3body;
	q3Scene* scene;
	
	Q3Wrapper(q3Scene* newscene, Movingbox* box, q3Body* body);
	void sync_q3();
	void sync_glm();
};

class Q3PhysicsEngine : public PhysicsEngine { public:
	q3Scene scene;
	
	Q3PhysicsEngine(World* world, float dt);
	virtual void tick();
};

#endif
