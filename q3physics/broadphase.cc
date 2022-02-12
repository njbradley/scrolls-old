#include "broadphase.h"
#include "conversions.h"
#include "physics.h"
#include "scrolls/blocks.h"
#include "scrolls/blockiter.h"
#include "scrolls/world.h"
#include "scrolls/debug.h"


BlockBroadPhase::BlockBroadPhase(q3Scene* nscene, World* nworld):
q3BroadPhase(nscene), scene(nscene), world(nworld) {
	q3BodyDef def;
	def.bodyType = eStaticBody;
	worldbody = scene->CreateBody(def);
}

BlockBroadPhase::~BlockBroadPhase() {
	
}

bool BlockBroadPhase::can_collide(Pixel* pix) {
	return pix != nullptr and pix->value != 0;
}

void BlockBroadPhase::add_contact(Pixel* pix1, Pixel* pix2) {
	Q3PhysicsBox* box1 = Q3PhysicsBox::from_pix(pix1);
	Q3PhysicsBox* box2 = Q3PhysicsBox::from_pix(pix2);
	m_manager->AddContact(&box1->qbox, &box2->qbox);
}

void BlockBroadPhase::add_pair(FreeBlock* free1, FreeBlock* free2) {
	if (free1 < free2) {
		freeblock_pairs.emplace(free1, free2);
	} else if (free2 < free1) {
		freeblock_pairs.emplace(free2, free1);
	}
}

void BlockBroadPhase::remove_pairs(FreeBlock* freeblock) {
	for (auto iter = freeblock_pairs.begin(); iter != freeblock_pairs.end();) {
		const pair<FreeBlock*,FreeBlock*>& freepair = *iter;
		if (freepair.first == freeblock or freepair.second == freeblock) {
			iter = freeblock_pairs.erase(iter);
		} else {
			iter ++;
		}
	}
}

void BlockBroadPhase::update_collision(Block* block1, Block* block2) {
	if (block1 == nullptr or block2 == nullptr
	or (block1->freecontainer == nullptr and block2->freecontainer == nullptr)) {
		return;
	}
	
	if (expandbox(block1->hitbox()).collide(expandbox(block2->hitbox()))) {
		if (block1->continues and block2->continues) {
			for (int i = 0; i < csize3; i ++) {
				for (int j = 0; j < csize3; j ++) {
					update_collision(block1->get(i), block2->get(j));
				}
			}
		} else if (block1->continues) {
			if (can_collide(block2->pixel)) {
				for (int i = 0; i < csize3; i ++) {
					update_collision(block1->get(i), block2);
				}
			}
		} else if (block2->continues) {
			if (can_collide(block1->pixel)) {
				for (int i = 0; i < csize3; i ++) {
					update_collision(block1, block2->get(i));
				}
			}
		} else {
			if (can_collide(block1->pixel) and can_collide(block2->pixel)) {
				add_contact(block1->pixel, block2->pixel);
			}
		}
	}
}

void BlockBroadPhase::find_freeblocks_around(FreeBlock* free, Block* block) {
	for (int x = -1; x < 2; x ++) {
		for (int y = -1; y < 2; y ++) {
			for (int z = -1; z < 2; z ++) {
				Hitbox box = block->freebox();
				box.position += ivec3(x,y,z) * block->scale;
				if (box.collide(free->box)) {
					Block* newblock = block->get_global(block->globalpos + ivec3(x,y,z) * block->scale, block->scale);
					if (newblock != nullptr) {
						for (FreeBlock* otherfree = newblock->freechild; otherfree != nullptr; otherfree = otherfree->next) {
							add_pair(free, otherfree);
							// if (otherfree > free or otherfree->fixed or !Q3PhysicsBody::from_block(otherfree)->body->IsAwake()) {
							// 	update_collision(free, otherfree);
							// }
						}
					}
				}
			}
		}
	}
}

void BlockBroadPhase::find_freeblocks_below(FreeBlock* free, Block* block) {
	if (block != nullptr and block->freebox().collide(free->box)) {
		for (FreeBlock* otherfree = block->freechild; otherfree != nullptr; otherfree = otherfree->next) {
			add_pair(free, otherfree);
			// if (otherfree > free or otherfree->fixed) {
			// 	update_collision(free, otherfree);
			// }
		}
		if (block->continues) {
			for (int i = 0; i < csize3; i ++) {
				find_freeblocks_below(free, block->children[i]);
			}
		}
	}
}

void BlockBroadPhase::update_freeblock(FreeBlock* freeblock) {
	HitboxIterable<FreePixelIterator<Block>> freeiter (freeblock->highparent, expandbox(freeblock->box));
	
	for (FreePixelIterator<Block>& iter : freeiter.bases) {
		update_collision(freeblock, iter.curblock);
	}
}

void BlockBroadPhase::update_pairs(FreeBlock* freeblock) {
	remove_pairs(freeblock);
	
	HitboxIterable<FreePixelIterator<Block>> freeiter (freeblock->highparent, expandbox(freeblock->box));
	
	for (FreePixelIterator<Block>& iter : freeiter.bases) {
		find_freeblocks_below(freeblock, iter.curblock);
	}
	
	Block* curblock = freeblock->highparent->parent;
	
	while (curblock != nullptr) {
		find_freeblocks_around(freeblock, curblock);
		curblock = curblock->parent;
	}
}

Hitbox BlockBroadPhase::expandbox(Hitbox box) {
	box.negbox -= grow_amount;
	box.posbox += grow_amount;
	return box;
}

void BlockBroadPhase::UpdatePairs() {
	double start = getTime();
	for (FreeBlock* free = world->allfreeblocks; free != nullptr; free = free->allfreeblocks) {
		if (Q3PhysicsBody::from_block(free)->body->IsAwake()) {
			update_freeblock(free);
		}
	}
	
	for (const pair<FreeBlock*,FreeBlock*> freepair : freeblock_pairs) {
		if (Q3PhysicsBody::from_block(freepair.first)->body->IsAwake() or Q3PhysicsBody::from_block(freepair.second)->body->IsAwake()) {
			update_collision(freepair.first, freepair.second);
		}
	}
	logger->log(6) << getTime() - start << " update pairs " << endl;
}

bool BlockBroadPhase::TestOverlap(q3Box* qbox1, q3Box* qbox2) const {
	Hitbox body1 = Hitbox(
		qbox1->body->GetTransform().position,
		vec3(0,0,0),
		vec3(0,0,0),
		qbox1->body->GetQuaternion()
	);
	Hitbox body2 = Hitbox(
		qbox2->body->GetTransform().position,
		vec3(0,0,0),
		vec3(0,0,0),
		qbox2->body->GetQuaternion()
	);
	Hitbox box1 = body1.transform_out(q3tobox(qbox1));
	Hitbox box2 = body2.transform_out(q3tobox(qbox2));
	box1.negbox -= grow_amount;
	box1.posbox += grow_amount;
	box2.negbox -= grow_amount;
	box2.posbox += grow_amount;
	// cout << " TESTING " << box1 << endl << box2 << endl;
	bool result = box1.collide(box2);
	// if (result) cout << " STILL COLLIDING " << qbox1 << ' ' << qbox2 << endl;
	return result;
}
