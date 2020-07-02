#ifndef CONTROLS
#define CONTROLS

// Include GLFW
#include <GLFW/glfw3.h>

#include "entity-predef.h"
#include "blocks-predef.h"
#include "items-predef.h"
#include "blockdata-predef.h"
#include "world-predef.h"
#include "menu-predef.h"
#include "blockphysics-predef.h"
#include "ui.h"



/*
// Initial position : on +Z
glm::vec3 position = glm::vec3( 25, 45, 5 );
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
*/
float initialFoV = 110.0f;


float speed = 80.0f; // 3 units / second
float mouseSpeed = 0.003f;

bool mouse;
int button;
double timeout;

double scroll;

void mouse_button_call(GLFWwindow* window, int nbutton, int action, int mods) {
	if (action == GLFW_PRESS) {
		mouse = true;
		button = nbutton;
		timeout = 0;
	} else if (action == GLFW_RELEASE) {
		mouse = false;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	scroll = yoffset;
}



Player::Player(World* newworld, vec3 pos):
	Entity(newworld, pos, vec3(-0.7,-2.5,-0.7), vec3(0.7,0.9,0.7)), inven(10), backpack(10) { // vec3(-0.8,-3.6,-0.8), vec3(-0.2,0,-0.2)
	glfwSetMouseButtonCallback(window, mouse_button_call);
	glfwSetScrollCallback(window, scroll_callback);
	if (newworld == nullptr) {
		cout << 61 << endl;
	}
	//char arr[] = {1,0,1,0,1,0,1,0}
	selitem = 0;
	//inven.add( ItemStack(Item(itemstorage->items["chips"]), 10) );
	//inven.add( ItemStack(Item(itemstorage->items["crafting"]), 1));
}

Player::Player(World* newworld, istream& ifile):
	Entity(newworld, vec3(0,0,0), vec3(-0.7,-2.5,-0.7), vec3(0.7,0.9,0.7)), inven(ifile), backpack(ifile) {
		ifile >> selitem;
		ifile >> position.x >> position.y >> position.z;
		ifile >> vel.x >> vel.y >> vel.z >> angle.x >> angle.y;
		ifile >> health >> flying >> autojump;
		glfwSetMouseButtonCallback(window, mouse_button_call);
		glfwSetScrollCallback(window, scroll_callback);
}

void Player::save_to_file(ostream& ofile) {
	inven.save_to_file(ofile);
	backpack.save_to_file(ofile);
	ofile << selitem << ' ';
	ofile << position.x << ' ' << position.y << ' ' << position.z << ' ';
	ofile << vel.x << ' ' << vel.y << ' ' << vel.z << ' ' << angle.x << ' ' << angle.y << ' ';
	ofile << health << ' ' << flying << ' ' << autojump << ' ';
}
	
mat4 Player::getViewMatrix(){
	return ViewMatrix;
}
mat4 Player::getProjectionMatrix(){
	return ProjectionMatrix;
}

void Player::raycast(Pixel** hit, vec3* hitpos, DisplayEntity** target_entity) {
	vec3 v = position;
	double tiny = 0.00001;
	double x = position.x;
	double y = position.y;
	double z = position.z;
	Block* target = world->raycast(&x, &y, &z, pointing.x + tiny, pointing.y + tiny, pointing.z + tiny, 8);
	//cout << "ksjdflajsdfklajsd" << endl;
	vector<DisplayEntity*> entities;
	get_nearby_entities(&entities);
	//cout << entities.size() << endl;
	int bx, by, bz;
	float dist;
	if (target != nullptr) {
		dist = glm::length(position-vec3(x,y,z));
	} else {
		dist = -1;
	}
	//cout << dist << endl;
	*target_entity = nullptr;
	*hit = nullptr;
	//print(pointing);
	for (DisplayEntity* entity : entities) {
		vec3 start = position-entity->position;
		//cout << entity << endl;
		//print(start);
		float dt = 0;
		bool ray_valid = true;
		for (int axis = 0; axis < 3 and ray_valid; axis ++) {
			float time;
			if (start[axis] < 0) {
				time = start[axis] * -1 / pointing[axis];
			} else if (start[axis] >= entity->block->scale) {
				time = (start[axis] - entity->block->scale) * -1 / pointing[axis] + 0.001f;
			} else {
				continue;
			}
			//cout << time << ' ';
			if ( time < 0 ) {
				ray_valid = false;
				continue;
			} else if (time > dt) {
				dt = time;
			}
		}// cout << endl;
		if (!ray_valid or glm::length(pointing*dt) > 8) {
			continue;
		}
		start += pointing*dt;
		//print (start);
		double ex = start.x;
		double ey = start.y;
		double ez = start.z;
		if (ex < 0 or ex >= entity->block->scale or ey < 0 or ey >= entity->block->scale or ez < 0 or ez >= entity->block->scale) {
			continue;
		}
		//cout << "made it" << endl;
		Block* start_block = entity->block->get_global(int(ex) - (ex<0), int(ey) - (ey<0), int(ez) - (ez<0), 1);
		//cout << start_block << endl;
		Block* newtarget = start_block->raycast(entity, &ex, &ey, &ez, pointing.x + tiny, pointing.y + tiny, pointing.z + tiny, 8-glm::length(pointing*dt));
		float newdist;
		if (newtarget != nullptr) {
			newdist = glm::length(position-entity->position-vec3(ex,ey,ez));
		} else {
			newdist = -1;
		}
		//cout << dist << ' ' <<  newdist << endl;
		if ((dist == -1 and newdist != -1) or (newdist < dist and newdist != -1)) {
			dist = newdist;
			target = newtarget;
			*target_entity = entity;
		}
	}
	if (dist != -1) {
		*hit = target->get_pix();
		//cout << int((*hit)->value) << endl;
		//cout << x << ' ' << y << ' ' << z << "---------" << endl;
		*hitpos = vec3(x,y,z);
	}
}

void Player::right_mouse() {
	//cout << "riht" << endl;
	if (timeout < 0) {
		return;
	}
	timeout = -0.2;

	vec3 hitpos;
	Pixel* pix;
	DisplayEntity* target_entity;
	raycast(&pix, &hitpos, &target_entity);
	Item* inhand = inven.get(selitem);
	
	bool shifting = glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS;
	
	if (!shifting and inhand->do_rcaction(world)) {
		inven.use(selitem);
		return;
	}
	
	if (pix == nullptr) {
		debugblock = nullptr;
		debugentity = nullptr;
		return;
	}
	if (target_entity != nullptr) {
		
	}
	
	double x = hitpos.x;
	double y = hitpos.y;
	double z = hitpos.z;
	
	
	int ox = (int)x - (x<0);
	int oy = (int)y - (y<0);
	int oz = (int)z - (z<0);
	x -= pointing.x*0.001;
	y -= pointing.y*0.001;
	z -= pointing.z*0.001;
	int dx = (int)x - (x<0) - ox;
	int dy = (int)y - (y<0) - oy;
	int dz = (int)z - (z<0) - oz;
	//cout << dx << ' ' << dy << ' ' << dz << endl;
	//world->set(item, (int)x - (x<0), (int)y - (y<0), (int)z - (z<0));
	//Pixel* pix = target->get_pix();
	if (blocks->blocks[pix->value]->rcaction != "null" and !shifting) {
		blocks->blocks[pix->value]->do_rcaction(pix);
	} else if (pix->physicsgroup != nullptr and !shifting and pix->physicsgroup->rcaction()) {
		cout << "did rcaction" << endl;
	} else {
		if (!inhand->isnull and inhand->data->onplace != nullptr) {
			Item* item = inven.get(selitem);
			if (!item->isnull) {
				CharArray* arr = item->data->onplace;
				if (arr != nullptr) {
					arr->place(world, (int)x - (x<0), (int)y - (y<0), (int)z - (z<0), dx, dy, dz);
				}
			}
			inven.use(selitem);
		} else {
			debugblock = pix;
			debugentity = target_entity;
		}
	
	}
}

void Player::left_mouse() {
	//cout << "lefftt" << endl;
	vec3 hitpos;
	Pixel* pix;
	DisplayEntity* target_entity;
	raycast(&pix, &hitpos, &target_entity);
	if (pix == nullptr) {
		return;
	}
	if (target_entity != nullptr) {
		if (timeout < 0) {
			return;
		}
		timeout = -0.8;
		//cout << "hit" << endl;
		if (!target_entity->immune) {
			target_entity->vel += pointing * 1.0f + vec3(0,5,0) + vel;
			target_entity->health -= 1;
			cout << target_entity->health << endl;
		}
		//target_entity->alive = false;
		return;
	}
	
	double x = hitpos.x;
	double y = hitpos.y;
	double z = hitpos.z;
	
	//cout << "skdfls " << int(pix->value) << endl;
	//cout << ':' << x << ' ' << y << ' ' << z << endl;
	
	Item* item = inven.get(selitem);
	string tool = "null";
	if (!item->isnull) {
		tool = item->data->tool;
	}
	double time_needed = item->dig_time(pix->get());
	if (timeout > time_needed) {
		if (tool != "null") {
			item->damage(time_needed);
		}
		char newitem;
		if (item->isnull) {
			newitem = Item::ondig_null(world, (int)x - (x<0), (int)y - (y<0), (int)z - (z<0));
		} else {
			newitem = item->ondig(world, (int)x - (x<0), (int)y - (y<0), (int)z - (z<0));
		}
		if (newitem != 0) {
			blocks->blocks[newitem]->droptable.drop(&inven, this, item);
		}
		// string item_name = blocks->blocks[newitem]->item;
		// if (item_name != "null") {
		// 	if (!inven.add(ItemStack(Item(itemstorage->items[item_name]), 1))) {
		// 		backpack.add(ItemStack(Item(itemstorage->items[item_name]), 1));
		// 	}
		// }
		timeout = 0;
	}
	//world->mark_render_update(pair<int,int>((int)position.x/world->chunksize - (position.x<0), (int)position.z/world->chunksize - (position.z<0)));
}

void Player::die() {
	const ivec3 dir_array[6] =    {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
	vector<ItemStack> items;
	for (ItemStack stack : inven.items) {
		if (!stack.item.isnull) {
			items.push_back(stack);
		}
	}
	ivec3 pos(position);
	while (world->get(pos.x, pos.y, pos.z) == 0) {
		pos.y --;
	}
	pos.y ++;
	for (int i = 0; i < items.size(); i ++) {
		ivec3 off;
		do {
			off = dir_array[rand()%6];
		} while (world->get(pos.x+off.x, pos.y+off.y, pos.z+off.z) != 0);
		pos += off;
		world->set(pos.x, pos.y, pos.z, blocks->names["chest"]);
	}
	
	if (menu == nullptr) {
		vector<string> options = {"Respawn"};
		menu = new SelectMenu("You Died", options, [&, pos, items] (string result) {
			Pixel* chest = world->get_global(pos.x, pos.y, pos.z, 1)->get_pix();
			if (chest->physicsgroup != nullptr) {
				for (ItemStack stack : items) {
					chest->physicsgroup->add_item(stack);
				}
			}
			if (result == "Respawn") {
				World* w = world;
				delete world->player;
				w->spawn_player();
			} else if (result == "Quit") {
				cout << "you dead" << endl;
			}
			delete menu;
			menu = nullptr;
		});
	}
}

void Player::mouse_button() {
	cout << "hello" << endl;
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		
	} else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		
	}
}

void Player::drop_ticks() {
	lastTime = glfwGetTime();
}

void Player::computeMatricesFromInputs(){

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	glfwSetCursorPos(window, screen_x/2, screen_y/2);

	// Compute new orientation
	angle += vec2(mouseSpeed * float(screen_x/2 - xpos ),  mouseSpeed * float( screen_y/2 - ypos ));
	//cout << horizontalAngle << ' ' << verticalAngle << endl;
	if (angle.y > 1.55f) {
		angle.y = 1.55f;
	}
	if (angle.y < -1.55) {
		angle.y = -1.55f;
	}
	if (angle.x > 6.28f) {
		angle.x = 0;
	}
	if (angle.x < 0) {
		angle.x = 6.28;
	}
	
	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(angle.y) * sin(angle.x),
		sin(angle.y),
		cos(angle.y) * cos(angle.x)
	);
	pointing = direction;
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(angle.x - 3.14f/2.0f),
		0,
		cos(angle.x - 3.14f/2.0f)
	);
	vec3 forward = -glm::vec3(
		-cos(angle.x - 3.14f/2.0f),
		0,
		sin(angle.x - 3.14f/2.0f)
	);
	
	
	// Up vector
	glm::vec3 up = glm::cross( right, forward );
	
	
	float nspeed = speed;
	if (flying) {
		nspeed *= 3;
	}
	
	if (!consts[4]) {
		nspeed /= 10;
	}
	
	// Move forward
	if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS){
		// vec3 dir_const = vec3(1,0,0)*(float)consts[0] + vec3(0,0,1)*(float)consts[2] + vec3(-1,0,0)*(float)consts[3] + vec3(0,0,-1)*(float)consts[5];
		// if (length(dir_const-forward) < 0.9f and (consts[0] or consts[2] or consts[3] or consts[5]) and autojump and not consts[6]) {
		// 	vel.y = 6;
		// 	if (glfwGetKey( window, GLFW_KEY_SPACE ) == GLFW_PRESS) {
		// 		vel.y = 10;
		// 	}
		// }
		vel += forward * deltaTime * nspeed;
	}
	// Move backward
	if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
		vel -= forward * deltaTime * nspeed;
	}
	// Strafe right
	if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
			vel += right * deltaTime * nspeed;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS){
			vel -= right * deltaTime * nspeed;
	}
	if (glfwGetKey( window, GLFW_KEY_SPACE ) == GLFW_PRESS){
		if (flying) {
			position += up * deltaTime * speed;
		} else if (consts[4]) {
			vel.y = 15;// = vec3(0,10,0);//up * deltaTime * speed;
		}
	}
	if (glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS){
		if (flying) {
			position -= up * deltaTime * speed;
		}
	}
	
	for (int i = 0; i < 10; i ++) {
		if (glfwGetKey(window, (i != 9) ? GLFW_KEY_1 + i : GLFW_KEY_0)) {
			selitem = i;
			break;
		}
	}
	
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		left_mouse();
		timeout += deltaTime;
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		right_mouse();
		timeout += deltaTime;
	} else {
		timeout = 0;
	}
	//mouse = false;
	
	
	
	if (scroll != 0) {
		selitem -= scroll;
		if (selitem < 0) {
			selitem = 0;
		} if (selitem > 9) {
			selitem = 9;
		}
		scroll = 0;
	}
	
	if (health <= 0) {
		die();
	}
	
	
	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.
	
	
	// Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), aspect_ratio, 0.1f, 1000.0f);
	// Camera matrix
	ViewMatrix       = glm::lookAt(
								position,           // Camera is here
								position+direction, // and looks here : at the same position, plus "direction"
								up                  // Head is up (set to 0,-1,0 to look upside-down)
						   );

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}

void Player::render_ui(MemVecs * uivecs) {
	///hearts
	float scale = 0.1f;
	int i;
	for (i = 0; i < health; i ++) {
		draw_icon(uivecs, 3, i*scale, 1-scale, scale, scale);
	}
	for (; i < 10; i ++) {
		draw_icon(uivecs, 2, i*scale, 1-scale, scale, scale);
	}
	inven.render(uivecs, -0.5f, -1.0f);
	draw_text(uivecs, "\\/", -0.45f+selitem*0.1f, -0.85f);
	Item* sel = inven.get(selitem);
	if (!sel->isnull) {
		draw_text(uivecs, sel->descript(), -0.25f, -0.7f);
	}
}

#endif
