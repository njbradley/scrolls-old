#include "physics.h"
#include "conversions.h"
#include "scrolls/blocks.h"
#include "scrolls/blockiter.h"
#include "scrolls/world.h"
#include "scrolls/tiles.h"
#include "scrolls/blockdata.h"




q3Body* wbody;

Q3PhysicsBody::Q3PhysicsBody(FreeBlock* free, q3Scene* nscene): PhysicsBody(free), scene(nscene) {
	q3BodyDef bodydef;
	bodydef.bodyType = eDynamicBody;
	bodydef.position = freeblock->box.global_center();
	bodydef.linearVelocity = freeblock->box.velocity;
	bodydef.angularVelocity = freeblock->box.angularvel;
	bodydef.angle = glm::angle(freeblock->box.rotation);
	bodydef.axis = glm::axis(freeblock->box.rotation);
	body = scene->CreateBody(bodydef);
	
	// add_box(Hitbox(vec3(0), vec3(-0.5), vec3(0.5)));
	
	for (Pixel* pix : free->iter()) {
		if (pix != 0) {
			Q3PhysicsBox* tmp = Q3PhysicsBox::from_pix(pix, wbody);
		}
	}
	
	// sync_q3();
}

Q3PhysicsBody::~Q3PhysicsBody() {
	scene->RemoveBody(body);
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
	// freeblock->box.change_center(vec3(0,0,0));
	freeblock->box.position = body->GetTransform().position;
	freeblock->box.rotation = body->GetQuaternion();
	if (freeblock->box.movable()) {
		freeblock->box.velocity = body->GetLinearVelocity();
		freeblock->box.angularvel = body->GetAngularVelocity();
	}
	freeblock->box.mass = body->GetMass();
	// freeblock->box.change_center(body->GetLocalCenter());
	freeblock->set_box(freeblock->box);
}


void Q3PhysicsBody::update() {
	
}

void Q3PhysicsBody::apply_impulse(vec3 impulse) {
	body->ApplyLinearImpulse(impulse);
}

void Q3PhysicsBody::apply_impulse(vec3 impulse, vec3 point) {
	body->ApplyLinearImpulseAtWorldPoint(impulse, point);
}

Q3PhysicsBody* Q3PhysicsBody::from_block(FreeBlock* free, q3Scene* scene) {
	if (free->physicsbody == nullptr) {
		return new Q3PhysicsBody(free, scene);
	} else {
		return (Q3PhysicsBody*) free->physicsbody;
	}
}




Q3PhysicsBox::Q3PhysicsBox(Pixel* pix, q3Body* worldbody): PhysicsBox(pix) {
	update();
	if (pixel->parbl->freecontainer == nullptr) {
		qbox.body = worldbody;
	} else {
		Q3PhysicsBody* body = (Q3PhysicsBody*) pixel->parbl->freecontainer->physicsbody;
		body->body->AddBox(&qbox);
	}
}

void Q3PhysicsBox::update() {
	Hitbox box = pixel->parbl->local_hitbox();
	if (pixel->parbl->freecontainer != nullptr) {
		box.position -= pixel->parbl->freecontainer->box.local_center();
	}
	box.negbox += tiny_thresh;
	box.posbox -= tiny_thresh;
	boxtoq3(box, &qbox);
	
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
PhysicsEngine(world, dt), scene(deltatime, &broadphase), broadphase(&scene, world) {
	wbody = broadphase.worldbody;
}

void Q3PhysicsEngine::tick() {
	double start = getTime();
	Q3PhysicsBody* body = nullptr;
	for (Tile* tile : world->tiles) {
		for (FreeBlock* free = tile->allfreeblocks; free != nullptr; free = free->allfreeblocks) {
			body = Q3PhysicsBody::from_block(free, &scene);
			body->sync_glm();
		}
	}
	
	double mid = getTime();
	scene.Step();
	
	// cout << getTime() - mid << " Step  " << mid - start << " sync " << endl;
}

EXPORT_PLUGIN(Q3PhysicsEngine);
