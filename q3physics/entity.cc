#include "entity.h"


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
	
void Q3Wrapper::sync_q3() {
	
}

void Q3Wrapper::sync_glm() {
	
}







Q3PhysicsEngine::Q3PhysicsEngine(World* world, float dt): PhysicsEngine(world, dt), scene(deltatime) {
	
}

void Q3PhysicsEngine::tick() {
	
	scene.Step();
}

EXPORT_PLUGIN(Q3PhysicsEngine);
