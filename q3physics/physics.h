#ifndef Q3PH_ENTITY
#define Q3PH_ENTITY

#include "classes.h"
#include "broadphase.h"
#include "scrolls/physics.h"

#include "q3.h"

class Q3PhysicsBody : public PhysicsBody { public:
	q3Body* body;
	q3Scene* scene;
	
	Q3PhysicsBody(FreeBlock* free, q3Scene* scene);
	virtual ~Q3PhysicsBody();
	void sync_q3();
	void sync_glm();
	
	virtual void update();
	
	virtual void apply_impulse(vec3 impulse);
	virtual void apply_impulse(vec3 impulse, vec3 point);
	
	static Q3PhysicsBody* from_block(FreeBlock* free, q3Scene* scene);
};

class Q3PhysicsBox : public PhysicsBox { public:
	q3Box qbox;
	
	Q3PhysicsBox(Pixel* pix, q3Body* worldbody);
	
  virtual void update();
	
	static Q3PhysicsBox* from_pix(Pixel* pix, q3Body* worldbody);
	static constexpr float tiny_thresh = 0.0f;
};

class Q3PhysicsEngine : public PhysicsEngine { public:
	PLUGIN_HEAD(Q3PhysicsEngine);
	q3Scene scene;
	BlockBroadPhase broadphase;
	
	Q3PhysicsEngine(World* world, float dt);
	virtual void tick();
};

#endif
