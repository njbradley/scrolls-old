#ifndef Q3_BROADPHASE
#define Q3_BROADPHASE

#include "classes.h"

// This is my workaround for q3 to make it work the scrolls world
// The main problem is that there are way to many static blocks,
// If I was to add all blocks in the world to the q3Scene, then
// everything would take forever. Instead, I made my own Broadphase
// object, which in q3 is used to find approximate collisions and
// return pairs of objects that collide. This way, I create boxes on
// the fly for any static blocks that are being collided with, and return
// pairs of those objects.

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
	
	bool can_collide(Pixel* pix);
	void add_contact(Pixel* pix1, Pixel* pix2);
	void find_freeblocks_around(FreeBlock* free, Block* block);
	void find_freeblocks_below(FreeBlock* free, Block* block);
	void update_collision(Block* block1, Block* block2);
	
	void update_freeblock(FreeBlock* free);
	// Generates the contact list. All previous contacts are returned to the allocator
	// before generation occurs.
	virtual void UpdatePairs( void );
	Hitbox expandbox(Hitbox box);
	
	virtual void Update( i32 id, const q3AABB& aabb ) { };

	virtual bool TestOverlap(q3Box* box1, q3Box* box2) const;
	
	static constexpr float grow_amount = 0.51f;
};

#endif
