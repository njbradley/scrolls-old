#include "broadphase.h"
#include "conversions.h"
#include "entity.h"
#include "scrolls/blocks.h"
#include "scrolls/blockiter.h"
#include "scrolls/world.h"
#include "scrolls/tiles.h"


BlockBroadPhase::BlockBroadPhase(q3Scene* nscene, World* nworld):
q3BroadPhase(nscene), scene(nscene), world(nworld) {
	cout << m_manager << " Manager " << endl;
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
				if (inpix->value != 0 and pixbox.collide(inpix->parbl->hitbox())) {
					cout << " PAIR " << pix << ' ' << inpix << endl;
					cout << "   " << pixbox << endl << "   " << inpix->parbl->hitbox() << endl;
					cout << "  " << q3tobox(&pixbod->qbox) << endl;
					q3Box* tmp = &Q3PhysicsBox::from_pix(inpix, worldbody)->qbox;
					cout << "  " << q3tobox(tmp) << endl;
					cout << pixbod->qbox.e << ' ' << tmp->e << endl;
					m_manager->AddContact(&pixbod->qbox, tmp);
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
	cout << " TESTING " << box1 << endl << box2 << endl;
	bool result = box1.collide(box2);
	if (result) cout << " STILL COLLIDING " << qbox1 << ' ' << qbox2 << endl;
	return result;
}
