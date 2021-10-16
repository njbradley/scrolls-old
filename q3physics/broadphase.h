#ifndef Q3_BROADPHASE
#define Q3_BROADPHASE

#include "classes.h"

class BlockBroadPhase : public q3BroadPhase {
public:
	q3Scene* scene;
	q3Body* worldbody;
	World* world;
	
	BlockBroadPhase(q3Scene* nscene, World* nworld);
	virtual ~BlockBroadPhase();
	
	// Unused because I am inserting boxes in here
	virtual void InsertBox( q3Box *shape, const q3AABB& aabb ) { };
	virtual void RemoveBox( const q3Box *shape ) { };
	
	void update_freeblock(FreeBlock* free);
	// Generates the contact list. All previous contacts are returned to the allocator
	// before generation occurs.
	virtual void UpdatePairs( void );

	virtual void Update( i32 id, const q3AABB& aabb ) { };

	virtual bool TestOverlap(q3Box* box1, q3Box* box2) const;
};

#endif
