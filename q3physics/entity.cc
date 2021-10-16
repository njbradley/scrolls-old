#include "entity.h"
#include "conversions.h"
#include "scrolls/blocks.h"
#include "scrolls/blockiter.h"
#include "scrolls/world.h"
#include "scrolls/tiles.h"
#include "scrolls/blockdata.h"






Q3PhysicsBody::Q3PhysicsBody(FreeBlock* free, q3Scene* scene): PhysicsBody(free) {
	q3BodyDef bodydef;
	bodydef.bodyType = eDynamicBody;
	bodydef.position = freeblock->box.position;
	bodydef.angle = glm::angle(freeblock->box.rotation);
	bodydef.axis = glm::axis(freeblock->box.rotation);
	body = scene->CreateBody(bodydef);
	
	// add_box(Hitbox(vec3(0), vec3(-0.5), vec3(0.5)));
	
	for (Pixel* pix : free->iter()) {
		if (pix != 0) {
			add_box(pix->parbl->local_hitbox());
		}
	}
	
	// sync_q3();
}

void Q3PhysicsBody::add_box(Hitbox box) {
	cout << box << endl;
	q3BoxDef boxdef;
	q3Transform transform;
	transform.position = box.point1();
	transform.rotation = glm::mat3_cast(box.rotation);
	
	boxdef.Set(transform, box.size());
	
	body->AddBox(boxdef);
}

void Q3PhysicsBody::sync_q3() {
	body->SetTransform(
		freeblock->box.position, glm::axis(freeblock->box.rotation),
		glm::angle(freeblock->box.rotation)
	);
	if (freeblock->box.movable()) {
		body->SetLinearVelocity(freeblock->box.velocity);
		body->SetAngularVelocity(freeblock->box.angularvel);
	}
}

void Q3PhysicsBody::sync_glm() {
	freeblock->box.position = body->GetTransform().position;
	freeblock->box.rotation = body->GetQuaternion();
	if (freeblock->box.movable()) {
		freeblock->box.velocity = body->GetLinearVelocity();
		freeblock->box.angularvel = body->GetAngularVelocity();
	}
}


void Q3PhysicsBody::update() {
	
}

Q3PhysicsBody* Q3PhysicsBody::from_block(FreeBlock* free, q3Scene* scene) {
	if (free->physicsbody == nullptr) {
		return new Q3PhysicsBody(free, scene);
	} else {
		return (Q3PhysicsBody*) free->physicsbody;
	}
}




Q3PhysicsBox::Q3PhysicsBox(Pixel* pix, q3Body* worldbody): PhysicsBox(pix) {
	if (pixel->parbl->freecontainer == nullptr) {
		qbox.body = worldbody;
	} else {
		Q3PhysicsBody* body = (Q3PhysicsBody*) pixel->parbl->freecontainer->physicsbody;
		qbox.body = body->body;
	}
}

void Q3PhysicsBox::update() {
	boxtoq3(pixel->parbl->local_hitbox(), &qbox);
	qbox.next = nullptr;
	
	// box.body = worldbody;
	qbox.friction = 0.4;
	qbox.restitution = 0.2;
	qbox.density = 1.0;
	qbox.sensor = false;
}

Q3PhysicsBox* Q3PhysicsBox::from_pix(Pixel* pix, q3Body* worldbody) {
	if (pix->physicsbox == nullptr) {
		return new Q3PhysicsBox(pix, worldbody);
	} else {
		return (Q3PhysicsBox*) pix->physicsbox;
	}
}






Q3PhysicsEngine::Q3PhysicsEngine(World* world, float dt):
PhysicsEngine(world, dt), scene(deltatime), broadphase(&scene, world) {
	cout << "Initialized physics " << endl;
}

void Q3PhysicsEngine::tick() {
	double start = getTime();
	
	for (Tile* tile : world->tiles) {
		for (FreeBlock* free = tile->allfreeblocks; free != nullptr; free = free->allfreeblocks) {
			Q3PhysicsBody* body = Q3PhysicsBody::from_block(free, &scene);
			body->sync_glm();
		}
	}
	
	double mid = getTime();
	scene.Step();
	
	cout << getTime() - mid << " Step  " << mid - start << " sync " << endl;
}

EXPORT_PLUGIN(Q3PhysicsEngine);
