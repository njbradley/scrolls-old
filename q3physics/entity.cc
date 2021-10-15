#include "entity.h"
#include "scrolls/blocks.h"
#include "scrolls/blockiter.h"
#include "scrolls/world.h"
#include "scrolls/tiles.h"
#include "scrolls/blockdata.h"





Q3Wrapper::Q3Wrapper(q3Scene* newscene, Movingbox* box, q3Body* body): scene(newscene), movingbox(box), q3body(body) {
	
}

Q3Wrapper::Q3Wrapper(q3Scene* newscene, Movingbox* box): scene(newscene), movingbox(box) {
	q3BodyDef bodydef;
	if (movingbox->movable()) {
		cout << "MOVABLE " << endl;
		bodydef.bodyType = eDynamicBody;
	} else {
		bodydef.bodyType = eStaticBody;
	}
	bodydef.position = box->position;
	bodydef.angle = glm::angle(movingbox->rotation);
	bodydef.axis = glm::axis(movingbox->rotation);
	q3body = scene->CreateBody(bodydef);
	// sync_q3();
}

void Q3Wrapper::add_box(Hitbox box) {
	q3BoxDef boxdef;
	q3Transform transform;
	transform.position = box.global_midpoint();
	transform.rotation = glm::mat3_cast(box.rotation);
	
	boxdef.Set(transform, box.size());
	
	q3body->AddBox(boxdef);
}

void Q3Wrapper::sync_q3() {
	q3body->SetTransform(movingbox->position, glm::axis(movingbox->rotation), glm::angle(movingbox->rotation));
	if (movingbox->movable()) {
		q3body->SetLinearVelocity(movingbox->velocity);
		q3body->SetAngularVelocity(movingbox->angularvel);
	}
}

void Q3Wrapper::sync_glm() {
	movingbox->position = q3body->GetTransform().position;
	movingbox->rotation = q3body->GetQuaternion();
	if (movingbox->movable()) {
		movingbox->velocity = q3body->GetLinearVelocity();
		movingbox->angularvel = q3body->GetAngularVelocity();
	}
}







Q3PhysicsEngine::Q3PhysicsEngine(World* world, float dt): PhysicsEngine(world, dt), scene(deltatime) {
	cout << "Initialized physics " << endl;
}

void Q3PhysicsEngine::on_tile_load(Tile* newtile) {
	cout << "loading tile " << newtile << ' ' << newtile->pos << endl;
	Q3Wrapper* tilebody = new Q3Wrapper(&scene, new Movingbox(newtile->chunk->movingbox()));
	int i = 0;
	for (Pixel* pix : newtile->chunk->iter()) {
		if (pix->value != 0 and pix->parbl->scale == 1 and pix->value == blocktypes::leaves.id) {
			tilebody->add_box(pix->parbl->local_hitbox());
			cout << i ++ << endl;
		}
	}
	tiles.push_back(tilebody);
	cout << " done loading " << endl;
}

void Q3PhysicsEngine::on_freeblock(FreeBlock* freeblock) {
	cout << "loading freeblock " << freeblock->box << endl;
	Q3Wrapper* body = new Q3Wrapper(&scene, &freeblock->box);
	
	for (Pixel* pix : freeblock->iter()) {
		if (pix->value != 0) {
			body->add_box(pix->parbl->local_hitbox());
		}
	}
	freeblocks.push_back(body);
	cout << " done loading " << endl;
}

void Q3PhysicsEngine::tick() {
	double start = getTime();
	for (Q3Wrapper* body : freeblocks) {
		body->sync_glm();
		// cout << body->q3body->GetTransform().position.y << ' ' << body->q3body->GetLinearVelocity() << endl;
	}
	double mid = getTime();
	scene.Step();
	
	cout << getTime() - mid << " Step  " << mid - start << " sync " << endl;
}

EXPORT_PLUGIN(Q3PhysicsEngine);
