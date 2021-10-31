#include "broadphase.h"
#include "conversions.h"
#include "entity.h"
#include "scrolls/blocks.h"
#include "scrolls/blockiter.h"
#include "scrolls/world.h"
#include "scrolls/tiles.h"
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
	Q3PhysicsBox* box1 = Q3PhysicsBox::from_pix(pix1, worldbody);
	Q3PhysicsBox* box2 = Q3PhysicsBox::from_pix(pix2, worldbody);
	m_manager->AddContact(&box1->qbox, &box2->qbox);
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

void BlockBroadPhase::update_freeblock(FreeBlock* freeblock) {
	FreeBlockIter freeiter (freeblock->highparent, expandbox(freeblock->box));
	
	for (int i = 0; i < freeiter.num_bases; i ++) {
		update_collision(freeblock, freeiter.bases[i]);
	}
	
	FreeBlockFreeIter freefreeiter (freeblock->highparent, expandbox(freeblock->box));
	
	for (FreeBlock* free : freefreeiter) {
		if (free > freeblock) {
			update_collision(freeblock, free);
		}
	}
	
	/*
	Hitbox fbox = freeblock->box;
	fbox.negbox -= grow_amount*2;
	fbox.posbox += grow_amount*2;
	
	/// freeblock to terrain
	FreeBlockIter iter (freeblock->highparent, fbox);
	for (Pixel* pix : iter) {
		if (pix->value != 0) {
			Hitbox pixbox = pix->parbl->hitbox();
			pixbox.negbox -= grow_amount;
			pixbox.posbox += grow_amount;
			Q3PhysicsBox* pixbod = Q3PhysicsBox::from_pix(pix, worldbody);
			
			// Hitbox inbox = freeblock->box.transform_in(pix->parbl->hitbox());
			// FreeBlockIter initer (freeblock, inbox);
			for (Pixel* inpix : freeblock->iter()) {
				Hitbox inbox = inpix->parbl->hitbox();
				inbox.negbox -= grow_amount;
				inbox.posbox += grow_amount;
				// cout << " TEST PAIR " << pixbox << endl << "  " << inbox << endl;
				if (inpix->value != 0 and pixbox.collide(inbox)) {
					// cout << " PAIR " << pix << ' ' << inpix << endl;
					// cout << "   " << pixbox << endl << "   " << inbox << endl;
					// cout << "  " << q3tobox(&pixbod->qbox) << endl;
					q3Box* tmp = &Q3PhysicsBox::from_pix(inpix, worldbody)->qbox;
					// cout << "  " << q3tobox(tmp) << endl;
					// cout << pixbod->qbox.e << ' ' << tmp->e << endl;
					m_manager->AddContact(&pixbod->qbox, tmp);
				}
			}
		}
	}
	
	// freeblock to other
	ivec3 tilepos = SAFEFLOOR3(freeblock->box.position / (float)World::chunksize);
	for (int i = -1; i < 2; i ++) {
		for (int j = -1; j < 2; j ++) {
			for (int k = -1; k < 2; k ++) {
				Tile* tile = world->tileat(tilepos+ivec3(i, j, k));
				if (tile != nullptr) {
					for (FreeBlock* free = tile->allfreeblocks; free != nullptr; free = free->allfreeblocks) {
						if (free > freeblock and expandbox(freeblock->box).collide(expandbox(free->box))) {
							for (Pixel* pix1 : freeblock->iter()) {
								for (Pixel* pix2 : free->iter()) {
									if (pix1->value != 0 and pix2->value != 0
									and expandbox(pix1->parbl->hitbox()).collide(expandbox(pix2->parbl->hitbox()))) {
										m_manager->AddContact(
											&Q3PhysicsBox::from_pix(pix1, worldbody)->qbox,
											&Q3PhysicsBox::from_pix(pix2, worldbody)->qbox
										);
									}
								}
							}
						}
					}
				}
			}
		}
	}*/
}

Hitbox BlockBroadPhase::expandbox(Hitbox box) {
	box.negbox -= grow_amount;
	box.posbox += grow_amount;
	return box;
}

void BlockBroadPhase::UpdatePairs() {
	double start = getTime();
	for (Tile* tile : world->tiles) {
		for (FreeBlock* free = tile->allfreeblocks; free != nullptr; free = free->allfreeblocks) {
			update_freeblock(free);
		}
	}
	// cout << getTime() - start << " update pairs " << endl;
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
