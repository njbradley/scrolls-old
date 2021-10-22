#include "conversions.h"
#include "scrolls/entity.h"
#include "scrolls/tests.h"

Hitbox q3tobox(q3Box* qbox1) {
	// cout << qbox1 << ' ' << qbox1->local.position
	Hitbox box1 (
		qbox1->local.position,
		-qbox1->e, qbox1->e,
		glm::quat_cast(glm::mat3(qbox1->local.rotation))
	);
	return box1;
}
	
void boxtoq3(Hitbox box, q3Box* qbox) {
	box.change_center(box.local_midpoint());
	qbox->local.position = box.position;
	qbox->local.rotation = glm::mat3_cast(box.rotation);
	qbox->e = box.size()/2.0f;
}



struct Q3ConversionTests : Test {
	Q3ConversionTests() {
		name = "Q3ConversionTests";
	}
	
	void test() {
		Hitbox box1 (vec3(0,0,0), vec3(-1,-1,-1), vec3(1,1,1), glm::angleAxis(1.0f, vec3(1,0,0)));
		q3Box qbox;
		boxtoq3(box1, &qbox);
		Hitbox box2 = q3tobox(&qbox);
		cout << box1 << endl;
		cout << box2 << endl;
		TASSERT_EQ(box1.point1(), box2.point1());
		TASSERT_EQ(box1.point2(), box2.point2());
	}
};

EXPORT_PLUGIN(Q3ConversionTests);
