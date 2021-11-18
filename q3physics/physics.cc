#include "physics.h"
#include "conversions.h"
#include "scrolls/blocks.h"
#include "scrolls/blockiter.h"
#include "scrolls/world.h"
#include "scrolls/tiles.h"
#include "scrolls/blockdata.h"
#include "scrolls/game.h"
#include "scrolls/materials.h"
#include "scrolls/debug.h"

#include "qu3e/dynamics/q3Contact.h"

// PLUGIN_REQUIRES(game->world->, physics, Q3PhysicsEngine);



PLUGIN_REQUIRES_RUNTIME(game->world->physics, physics, Q3PhysicsEngine);

Q3PhysicsBody::Q3PhysicsBody(FreeBlock* free): PhysicsBody(free) {
	q3BodyDef bodydef;
	bodydef.bodyType = eDynamicBody;
	bodydef.position = freeblock->box.position;
	bodydef.linearVelocity = freeblock->box.velocity;
	bodydef.angularVelocity = freeblock->box.angularvel;
	bodydef.angle = glm::angle(freeblock->box.rotation);
	bodydef.axis = glm::axis(freeblock->box.rotation);
	bodydef.lockAxisX = !freeblock->allow_rotation;
	bodydef.lockAxisY = !freeblock->allow_rotation;
	bodydef.lockAxisZ = !freeblock->allow_rotation;
	bodydef.allowSleep = freeblock->allow_sleep;
	body = physics->scene.CreateBody(bodydef);
	
	// add_box(Hitbox(vec3(0), vec3(-0.5), vec3(0.5)));
	
	for (Pixel* pix : free->iter()) {
		if (pix != 0) {
			Q3PhysicsBox* tmp = Q3PhysicsBox::from_pix(pix);
		}
	}
	
	// sync_q3();
}

Q3PhysicsBody::~Q3PhysicsBody() {
	physics->scene.RemoveBody(body);
}

void Q3PhysicsBody::sync_q3(float curtime, float dt) {
	body->SetTransform(
		freeblock->box.position, glm::axis(freeblock->box.rotation),
		glm::angle(freeblock->box.rotation)
	);
	if (freeblock->box.movable()) {
		body->SetLinearVelocity(freeblock->box.velocity);
		body->SetAngularVelocity(freeblock->box.angularvel);
	}
}

void Q3PhysicsBody::sync_glm(float curtime, float dt) {
	// freeblock->box.change_center(vec3(0,0,0));
	Movingbox newbox = freeblock->box;
	newbox.position = body->GetTransform().position;
	newbox.rotation = body->GetQuaternion();
	if (freeblock->box.movable()) {
		newbox.velocity = body->GetLinearVelocity();
		newbox.angularvel = body->GetAngularVelocity();
	}
	newbox.mass = body->GetMass();
	// freeblock->box.change_center(body->GetLocalCenter());
	freeblock->set_box(newbox, curtime);
}


void Q3PhysicsBody::update() {
	
}

void Q3PhysicsBody::apply_impulse(vec3 impulse) {
	body->ApplyLinearImpulse(impulse);
}

void Q3PhysicsBody::apply_impulse(vec3 impulse, vec3 point) {
	body->ApplyLinearImpulseAtWorldPoint(impulse, point);
}

vec3 Q3PhysicsBody::closest_contact(vec3 dir) {
	vec3 max_dir (0,0,0);
	float max_dot = 0;
	q3ContactEdge* edge = body->GetContactList();
	while (edge != nullptr) {
		
		q3Contact* contacts = edge->constraint->manifold.contacts;
		int num_contacts = edge->constraint->manifold.contactCount;
		
		vec3 newdir (0,0,0);
		
		if (num_contacts == 1) {
			newdir = glm::normalize(contacts[0].position - body->GetTransform().position);
		} else if (num_contacts == 2) {
			vec3 between_contacts = glm::normalize(contacts[1].position - contacts[0].position);
			vec3 from_center = contacts[0].position - body->GetTransform().position;
			from_center -= glm::dot(from_center, between_contacts) * between_contacts;
			newdir = glm::normalize(from_center);
		} else if (num_contacts >= 3) {
			newdir = glm::normalize(glm::cross(contacts[1].position - contacts[0].position, contacts[2].position - contacts[1].position));
			if (glm::dot(newdir, contacts[0].position - body->GetTransform().position) < 0) {
				newdir *= -1;
			}
		}
		
		if (num_contacts > 0) {
			debuglines->render(contacts[0].position, contacts[0].position - newdir);
		}
		
		if (glm::dot(newdir, dir) > max_dot) {
			max_dir = newdir;
			max_dot = glm::dot(newdir, dir);
		}
		
		//
		// for (int i = 0; i < edge->constraint->manifold.contactCount; i ++) {
		// 	vec3 contactpos = edge->constraint->manifold.contacts[i].position;
		// 	debuglines->render(contactpos, body->GetTransform().position);
		// 	vec3 newdir = glm::normalize(contactpos - body->GetTransform().position);
		// 	if (glm::dot(newdir, dir) > max_dot) {
		// 		max_dir = newdir;
		// 		max_dot = glm::dot(newdir, dir);
		// 	}
		// }
		edge = edge->next;
	}
	return max_dir;
}

bool Q3PhysicsBody::in_air() {
	return body->GetContactList() == nullptr;
}


Q3PhysicsBox::Q3PhysicsBox(Pixel* pix): PhysicsBox(pix) {
	update();
	if (pixel->parbl->freecontainer == nullptr) {
		qbox.body = physics->worldbody();
	} else {
		Q3PhysicsBody* body = Q3PhysicsBody::from_block(pixel->parbl->freecontainer);
		body->body->AddBox(&qbox);
	}
}

Q3PhysicsBox::~Q3PhysicsBox() {
	if (pixel->parbl->freecontainer != nullptr) {
		Q3PhysicsBody* body = Q3PhysicsBody::from_block(pixel->parbl->freecontainer);
		body->body->RemoveBoxNoFree(&qbox);
	} else {
		physics->worldbody()->RemoveContacts(&qbox);
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
	Material* mat = blockstorage[pixel->value]->material;
	qbox.friction = mat->friction;
	qbox.restitution = mat->restitution;
	qbox.density = mat->density;
	qbox.sensor = false;
}






Q3PhysicsEngine::Q3PhysicsEngine(World* world, float dt):
PhysicsEngine(world, dt), scene(deltatime, &broadphase, vec3(0,-9.8*2,0)), broadphase(&scene, world) {
	
}

q3Body* Q3PhysicsEngine::worldbody() const {
	return broadphase.worldbody;
}

void Q3PhysicsEngine::tick(float curtime, float dt) {
	double start = getTime();
	Q3PhysicsBody* body = nullptr;
	// for (Tile* tile : world->tiles) {
	// 	for (FreeBlock* free = tile->allfreeblocks; free != nullptr; free = free->allfreeblocks) {
	// 		body = Q3PhysicsBody::from_block(free);
	// 		body->sync_glm();
	// 	}
	// }
	
	double mid = getTime();
	scene.Step();
	
	for (Tile* tile : world->tiles) {
		for (FreeBlock* free = tile->allfreeblocks; free != nullptr; free = free->allfreeblocks) {
			body = Q3PhysicsBody::from_block(free);
			body->sync_glm(curtime, dt);
		}
	}
	// cout << getTime() - mid << " Step  " << mid - start << " sync " << endl;
}

EXPORT_PLUGIN(Q3PhysicsEngine);
