#include "entity.h"
#include "scrolls/blocks.h"
#include "scrolls/blockiter.h"
#include "scrolls/world.h"
#include "scrolls/tiles.h"
#include "scrolls/blockdata.h"

vec3 q3toglm(q3Vec3 pos) {
	return vec3(pos.x, pos.y, pos.z);
}

quat q3toglm(q3Quaternion rot) {
	return quat(rot.w, rot.x, rot.y, rot.z);
}

glm::mat3 q3toglm(q3Mat3 mat) {
	return {
		q3toglm(mat[0]),
		q3toglm(mat[1]),
		q3toglm(mat[2])
	};
}


q3Vec3 glmtoq3(vec3 pos) {
	return q3Vec3(pos.x, pos.y, pos.z);
}

q3Quaternion glmtoq3(quat rot) {
	return q3Quaternion(rot.x, rot.y, rot.z, rot.w);
}

q3Mat3 glmtoq3(glm::mat3 mat) {
	return q3Mat3(
		glmtoq3(mat[0]),
		glmtoq3(mat[1]),
		glmtoq3(mat[2])
	);
}




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
	bodydef.position = glmtoq3(box->position);
	bodydef.angle = glm::angle(movingbox->rotation);
	bodydef.axis = glmtoq3(glm::axis(movingbox->rotation));
	q3body = scene->CreateBody(bodydef);
	// sync_q3();
}

void Q3Wrapper::add_box(Hitbox box) {
	q3BoxDef boxdef;
	q3Transform transform;
	transform.position = glmtoq3(box.global_midpoint());
	transform.rotation = glmtoq3(glm::mat3_cast(box.rotation));
	
	boxdef.Set(transform, glmtoq3(box.size()));
	
	q3body->AddBox(boxdef);
}

void Q3Wrapper::sync_q3() {
	q3body->SetTransform(glmtoq3(movingbox->position), glmtoq3(glm::axis(movingbox->rotation)), glm::angle(movingbox->rotation));
	if (movingbox->movable()) {
		q3body->SetLinearVelocity(glmtoq3(movingbox->velocity));
		q3body->SetAngularVelocity(glmtoq3(movingbox->angularvel));
	}
}

void Q3Wrapper::sync_glm() {
	movingbox->position = q3toglm(q3body->GetTransform().position);
	movingbox->rotation = q3toglm(q3body->GetQuaternion());
	if (movingbox->movable()) {
		movingbox->velocity = q3toglm(q3body->GetLinearVelocity());
		movingbox->angularvel = q3toglm(q3body->GetAngularVelocity());
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
	
	for (Q3Wrapper* body : freeblocks) {
		body->sync_glm();
		cout << body->q3body->GetTransform().position.y << ' ' << q3toglm(body->q3body->GetLinearVelocity()) << endl;
	}
	
	scene.Step();
}

EXPORT_PLUGIN(Q3PhysicsEngine);
