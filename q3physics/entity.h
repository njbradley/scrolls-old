#ifndef Q3PH_ENTITY
#define Q3PH_ENTITY

#include "scrolls/classes.h"
#include "scrolls/entity.h"

#include "q3.h"


class Q3Wrapper { public:
	Movingbox* movingbox;
	q3Body* q3body;
	q3Scene* scene;
	
	Q3Wrapper(q3Scene* newscene, Movingbox* box, q3Body* body);
	Q3Wrapper(q3Scene* newscene, Movingbox* box);
	void add_box(Hitbox box);
	void sync_q3();
	void sync_glm();
};

class Q3PhysicsEngine : public PhysicsEngine { public:
	q3Scene scene;
	
	vector<Q3Wrapper*> tiles;
	vector<Q3Wrapper*> freeblocks;
	
	Q3PhysicsEngine(World* world, float dt);
	virtual void on_tile_load(Tile* tile);
	virtual void on_freeblock(FreeBlock* freeblock);
	virtual void tick();
};

#endif
