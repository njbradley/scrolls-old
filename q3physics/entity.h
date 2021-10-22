#ifndef Q3PH_ENTITY
#define Q3PH_ENTITY

#include "classes.h"
#include "broadphase.h"
#include "scrolls/entity.h"

#include "q3.h"

class Q3PhysicsBody : public PhysicsBody { public:
	q3Body* body;
	
	Q3PhysicsBody(FreeBlock* free, q3Scene* scene);
	void add_box(Hitbox box);
	void sync_q3();
	void sync_glm();
	
	virtual void update();
	
	static Q3PhysicsBody* from_block(FreeBlock* free, q3Scene* scene);
};

class Q3PhysicsBox : public PhysicsBox { public:
	q3Box qbox;
	
	Q3PhysicsBox(Pixel* pix, q3Body* worldbody);
	
  virtual void update();
	
	static Q3PhysicsBox* from_pix(Pixel* pix, q3Body* worldbody);
	static constexpr float tiny_thresh = 0.001f;
};

class Q3PhysicsEngine : public PhysicsEngine { public:
	q3Scene scene;
	BlockBroadPhase broadphase;
	
	vector<Q3Wrapper*> tiles;
	vector<Q3Wrapper*> freeblocks;
	
	Q3PhysicsEngine(World* world, float dt);
	virtual void tick();
};

#endif
