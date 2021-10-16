#include "broadphase.h"
#include "conversions.h"
#include "entity.h"
#include "scrolls/blocks.h"
#include "scrolls/blockiter.h"
#include "scrolls/world.h"
#include "scrolls/tiles.h"


BlockBroadPhase::BlockBroadPhase(q3Scene* nscene, World* nworld):
scene(nscene), world(nworld), q3BroadPhase(scene) {
	q3BodyDef def;
	def.bodyType = eStaticBody;
	worldbody = scene->CreateBody(def);
}

BlockBroadPhase::~BlockBroadPhase() {
	
}


void BlockBroadPhase::update_freeblock(FreeBlock* freeblock) {
	FreeBlockIter iter (freeblock->highparent, freeblock->box);
	for (Pixel* pix : iter) {
		if (pix->value != 0) {
			Hitbox pixbox = pix->parbl->hitbox();
			Q3PhysicsBox* pixbod = Q3PhysicsBox::from_pix(pix, worldbody);
			
			// Hitbox inbox = freeblock->box.transform_in(pix->parbl->hitbox());
			// FreeBlockIter initer (freeblock, inbox);
			for (Pixel* inpix : freeblock->iter()) {
				if (pixbox.collide(inpix->parbl->hitbox())) {
					m_manager->AddContact(&pixbod->qbox, &Q3PhysicsBox::from_pix(pix, worldbody)->qbox);
				}
			}
		}
	}
}

void BlockBroadPhase::UpdatePairs() {
	for (Tile* tile : world->tiles) {
		for (FreeBlock* free = tile->allfreeblocks; free != nullptr; free = free->allfreeblocks) {
			update_freeblock(free);
		}
	}
}

bool BlockBroadPhase::TestOverlap(q3Box* qbox1, q3Box* qbox2) const {
	Hitbox box1 = q3tobox(qbox1);
	Hitbox box2 = q3tobox(qbox2);
	return box1.collide(box2);
}
