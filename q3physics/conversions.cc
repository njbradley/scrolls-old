#include "conversions.h"
#include "scrolls/entity.h"

Hitbox q3tobox(q3Box* qbox1) {
	// cout << qbox1 << ' ' << qbox1->local.position 
	Hitbox box1 (
		qbox1->local.position,
		vec3(0,0,0), qbox1->e,
		glm::quat_cast(glm::mat3(qbox1->local.rotation))
	);
	return box1;
}
	
void boxtoq3(Hitbox box, q3Box* qbox) {
	box.change_center(vec3(0,0,0));
	qbox->local.position = box.position;
	qbox->local.rotation = glm::mat3_cast(box.rotation);
	qbox->e = box.size();
}
