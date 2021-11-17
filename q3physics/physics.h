#ifndef Q3PH_ENTITY
#define Q3PH_ENTITY

#include "classes.h"
#include "broadphase.h"
#include "scrolls/physics.h"

#include "q3.h"

class Q3PhysicsBody : public PhysicsBody { public:
	q3Body* body;
	
	Q3PhysicsBody(FreeBlock* free);
	virtual ~Q3PhysicsBody();
	void sync_q3(float curtime, float dt);
	void sync_glm(float curtime, float dt);
	
	virtual void update();
	
	virtual void apply_impulse(vec3 impulse);
	virtual void apply_impulse(vec3 impulse, vec3 point);
	
	template <typename ... Args>
	static Q3PhysicsBody* from_block(FreeBlock* free, Args ... args);
};

class Q3PhysicsBox : public PhysicsBox { public:
	q3Box qbox;
	
	Q3PhysicsBox(Pixel* pix);
	virtual ~Q3PhysicsBox();
	
  virtual void update();
	
	template <typename ... Args>
	static Q3PhysicsBox* from_pix(Pixel* pix, Args ... args);
	static constexpr float tiny_thresh = 0.0f;
};

class Q3PhysicsEngine : public PhysicsEngine { public:
	PLUGIN_HEAD(Q3PhysicsEngine);
	q3Scene scene;
	BlockBroadPhase broadphase;
	
	Q3PhysicsEngine(World* world, float dt);
	q3Body* worldbody() const;
	virtual void tick(float curtime, float deltatime);
};



#include "scrolls/blocks.h"

template <typename ... Args>
Q3PhysicsBody* Q3PhysicsBody::from_block(FreeBlock* free, Args ... args) {
	if (free->physicsbody == nullptr) {
		return new Q3PhysicsBody(free, args...);
	} else {
		return (Q3PhysicsBody*) free->physicsbody;
	}
}

template <typename ... Args>
Q3PhysicsBox* Q3PhysicsBox::from_pix(Pixel* pix, Args ... args) {
	if (pix->physicsbox == nullptr) {
		return new Q3PhysicsBox(pix, args...);
	} else {
		return (Q3PhysicsBox*) pix->physicsbox;
	}
}



#endif
