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
		ifile >> health >> max_health >> damage_health >> healing_health >> healing_speed;
		ifile >> flying >> autojump;
		glfwSetMouseButtonCallback(window, mouse_button_call);
		glfwSetScrollCallback(window, scroll_callback);
}

void Player::save_to_file(ostream& ofile) {
	inven.save_to_file(ofile);
	backpack.save_to_file(ofile);
	ofile << selitem << ' ';
	ofile << position.x << ' ' << position.y << ' ' << position.z << ' ';
	ofile << vel.x << ' ' << vel.y << ' ' << vel.z << ' ' << angle.x << ' ' << angle.y << ' ';
	ofile << health << ' ' << max_health << ' ' << damage_health << ' ' << healing_health << ' ' << healing_speed << ' ';
	ofile << flying << ' ' << autojump << ' ';
}
	
mat4 Player::getViewMatrix(){
	return ViewMatrix;
}
mat4 Player::getProjectionMatrix(){
	return ProjectionMatrix;
}

//
// void Player::calc_constraints() {
//     //int begin = clock();
//     vec3 coords_array[3] = {{0,1,1}, {1,0,1}, {1,1,0}};
//     vec3 dir_array[3] =    {{1,0,0}, {0,1,0}, {0,0,1}};
//
//     vec3 newpos(position);
//
//     bool new_consts[7];
//     for (int i = 0; i < 7; i ++) {
//       new_consts[i] = false;
//     }
//
//     vector<Collider*> colliders;
//     find_colliders(&colliders);
//
//     vec3 newbox2 = box2 - 1.0f - axis_gap;
//     vec3 newbox1 = box1 + axis_gap;
//
//     //box2 = vec3(0,0,0);
//
//     //this system works by creating a slice of blocks that define each side of collision. the sides all share either the most positive point or the most negative point
//     //the shlice is tested to see if any blocks are int it (2) and if there are blocks, the players location is rounded to outside the block (3)
//     //axis gap make sure a side detects the collision of blocks head on before the side detectors sense the blocks. Each slice is moved out from the player by axis_gap
//     //cout << "----------------------------------" << endl;
//     //print(position);
//
//
//
//     for (Collider* collider : colliders) {
//       vec3 rel_pos = position - collider->get_position();
//       vec3 new_rel_pos = newpos - collider->get_position();
//       for (int axis = 0; axis < 3; axis ++) {
//         vec3 coords = coords_array[axis];
//         vec3 dir = dir_array[axis];
//
//         // pos is the positive point of the hitbox rounded up to the nearest block. neg is the negative most point rounded down
//         vec3 neg = floor( rel_pos + newbox1 - axis_gap*dir );// + vec3(0,1,0);
//         vec3 pos = ceil( rel_pos + newbox2 + axis_gap*dir );
//         if (axis != 1 and !consts[2]) {
//           neg = floor( rel_pos + newbox1 - axis_gap*dir + vec3(0,1,0));
//         }
//
//         //positive side
//         //pos2 is the other point for the face
//         vec3 pos2 = pos*dir + neg*coords;
//         bool constraint = false;
//         for (int x = (int)pos2.x; x < pos.x+1; x ++) {
//             for (int y = (int)pos2.y; y < pos.y+1; y ++) {
//                 for (int z = (int)pos2.z; z < pos.z+1; z ++) {
//                     Block* pix = collider->get_global(x,y,z,1);
//                     bool new_const = (pix != NULL and pix->get() != 0);
//                     if (new_const) {
//                       //world->block_update(x,y,z);
//                     }
//                     constraint = constraint or new_const;
//                 }
//             }
//         }
//         new_consts[axis] = new_consts[axis] or constraint;
//         if (constraint) {
//             new_rel_pos[axis] = floor(rel_pos[axis] + newbox2[axis] + axis_gap) - newbox2[axis] - axis_gap;
//         }
//         vec3 neg2 = neg*dir + pos*coords;
//         if (axis == 1) {
//           //neg -= vec3(0.5,0,0.5);
//           //neg2 += vec3(0.5,0,0.5);
//           int max_y = INT_MIN;
//           bool max_y_set = false;
//           constraint = false;
//           for (int x = (int)neg.x; x < neg2.x+1; x ++) {
//               for (int y = (int)neg.y; y < neg2.y+1; y ++) {
//                   for (int z = (int)neg.z; z < neg2.z+1; z ++) {
//                       Block* pix = collider->get_global(x,y,z,1);
//                       if (pix == nullptr or pix->get() == 0) {
//                         continue;
//                       }
//                       constraint = true;
//                       for (int yoff = 1; yoff < 3; yoff ++) {
//                         pix = collider->get_global(x,y+yoff,z,1);
//                         if (pix == nullptr or pix->get() == 0) {
//                           if (y+yoff-1 > max_y) {
//                             max_y = y+yoff-1;
//                             max_y_set = true;
//                           }
//                           constraint = true;
//                           break;
//                         }
//                       }
//                   }
//               }
//           }
//
//           if (!constraint) {
//             neg = floor( rel_pos + newbox1 - axis_gap );
//             pos = ceil( rel_pos + newbox2 + axis_gap );
//             neg2 = neg*dir + pos*coords;
//
//             for (int x = (int)neg.x; x < neg2.x+1; x ++) {
//               for (int y = (int)neg.y; y < neg2.y+1; y ++) {
//                 for (int z = (int)neg.z; z < neg2.z+1; z ++) {
//                   Block* floor = collider->get_global(x,y,z,1);
//                   if (floor != nullptr and floor->get() != 0) {
//                     Block* above = collider->get_global(x,y+1,z,1);
//                     if (above == nullptr or above->get() == 0) {
//                       constraint = true;
//                     }
//                   }
//
//                 }
//               }
//             }
//
//             if (constraint) {
//               float newpos = ceil(rel_pos[axis] + newbox1[axis] - axis_gap) - newbox1[axis] + axis_gap;
//               if (std::abs(newpos - new_rel_pos[axis]) > 0.3) {
//                 constraint = false;
//               } else {
//                 new_rel_pos[axis] = newpos;
//               }
//             }
//
//           } else {
//             if (constraint and max_y_set) {
//               double newpos = max_y - newbox1[axis] + axis_gap + 1;
//               if (newpos > new_rel_pos[1]) {
//                 double diff = newpos - new_rel_pos[1];
//                 if (diff > 0.2) {
//                   diff = 0.2;
//                 }
//                 new_rel_pos[1] += diff;
//               } else {
//                 new_consts[axis+3] = false;
//               }
//               //ceil(rel_pos[axis] + newbox1[axis] - axis_gap) - newbox1[axis] + axis_gap;
//             }
//           }
//
//           new_consts[axis+3] = new_consts[axis+3] or constraint;
//
//         } else {
//           constraint = false;
//           for (int x = (int)neg.x; x < neg2.x+1; x ++) {
//               for (int y = (int)neg.y; y < neg2.y+1; y ++) {
//                   for (int z = (int)neg.z; z < neg2.z+1; z ++) {
//                       Block* pix = collider->get_global(x,y,z,1);
//                       bool new_const = (pix != NULL and pix->get() != 0);
//                       if (new_const) {
//                         //world->block_update(x,y,z);
//                       }
//                       constraint = constraint or new_const;
//                   }
//               }
//           }
//           new_consts[axis+3] = new_consts[axis+3] or constraint;
//           if (constraint) {
//             new_rel_pos[axis] = ceil(rel_pos[axis] + newbox1[axis] - axis_gap) - newbox1[axis] + axis_gap;
//           }
//         }
//       }
//       /// torso check: to tell if the constraints are only on the feet
//
//       vec3 neg = floor( rel_pos + newbox1 - axis_gap + vec3(0,1.2,0)) ;
//       vec3 pos = ceil( rel_pos + newbox2 + axis_gap );
//       bool constraint = false;
//       for (int x = (int)neg.x; x < pos.x+1; x ++) {
//         for (int y = (int)neg.y; y < pos.y+1; y ++) {
//           for (int z = (int)neg.z; z < pos.z+1; z ++) {
//             Block* pix = collider->get_global(x,y,z,1);
//             constraint = constraint or (pix != NULL and pix->get() != 0);
//           }
//         }
//       }
//       new_consts[6] =  new_consts[6] or constraint;
//
//       newpos = new_rel_pos + collider->get_position();
//     }
//     for (int i = 0; i < 7; i ++) {
//         consts[i] = old_consts[i] or new_consts[i];
//         old_consts[i] = new_consts[i];
//     }
//
//     position = newpos;
//     //cout << clock() - begin << endl;
//     //print(position);
// }

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
		*hit = target->get_pix();
		*hitpos = vec3(x,y,z);
	} else {
		dist = -1;
		*hit = nullptr;
	}
	//cout << dist << endl;
	*target_entity = nullptr;
	//print(pointing);
	for (DisplayEntity* entity : entities) {
		vec3 start = position-entity->position;
		//cout << entity << endl;
		//print(start);
		float dt = 0;
		float maxdt = -1;
		bool ray_valid = true;
		for (int axis = 0; axis < 3 and ray_valid; axis ++) {
			float mintime;
			float maxtime;
			
			if (start[axis] < entity->box1[axis]) {
				mintime = (start[axis] - entity->box1[axis]) * -1 / pointing[axis];
				maxtime = (start[axis] - entity->box2[axis]) * -1 / pointing[axis];
			} else if (start[axis] > entity->box2[axis]) {
				mintime = (start[axis] - entity->box2[axis]) * -1 / pointing[axis];
				maxtime = (start[axis] - entity->box1[axis]) * -1 / pointing[axis];
			} else {
				mintime = 0;
				if (pointing[axis] < 0) {
					maxtime = (start[axis] - entity->box1[axis]) * -1 / pointing[axis];
				} else {
					maxtime = (start[axis] - entity->box2[axis]) * -1 / pointing[axis];
				}
			}
			if (maxdt == -1) {
				maxdt = maxtime;
			}
			if ( mintime < 0 or mintime > maxdt or maxtime < dt) {
				ray_valid = false;
				break;
			} else {
				if (mintime > dt) {
					dt = mintime;
				}
				if (maxtime < maxdt) {
					maxdt = maxtime;
				}
			}
		}
		if (!ray_valid or glm::length(pointing*dt) > 8) {
			continue;
		}
		start += pointing*dt;
		//if (start.x > 0 and start.x <= entity->box2.x and start.y > 0 and start.y <= entity->box2.y and start.z < 0 and start.z <= entity->box2.z) {
			
		//print (start);
		
			float newdist = glm::length(pointing*dt);
			if (dist == -1 or newdist < dist) {
				dist = newdist;
				*target_entity = entity;
				*hitpos = entity->position + start;
			}
		//}
	}
	// 	//cout << "made it" << endl;
	// 	Block* start_block = entity->block->get_global(int(ex) - (ex<0), int(ey) - (ey<0), int(ez) - (ez<0), 1);
	// 	//cout << start_block << endl;
	// 	Block* newtarget = start_block->raycast(entity, &ex, &ey, &ez, pointing.x + tiny, pointing.y + tiny, pointing.z + tiny, 8-glm::length(pointing*dt));
	// 	float newdist;
	// 	if (newtarget != nullptr) {
	// 		newdist = glm::length(position-entity->position-vec3(ex,ey,ez));
	// 	} else {
	// 		newdist = -1;
	// 	}
	// 	//cout << dist << ' ' <<  newdist << endl;
	// 	if ((dist == -1 and newdist != -1) or (newdist < dist and newdist != -1)) {
	// 		dist = newdist;
	// 		target = newtarget;
	// 		*target_entity = entity;
	// 	}
	// }
	
	
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
	} else if (pix->physicsgroup != nullptr and !shifting and pix->physicsgroup->rcaction(this, inhand)) {
		
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
	if (target_entity != nullptr) {
		if (timeout < 0) {
			return;
		}
		timeout = -0.8;
		//cout << "hit" << endl;
		if (!target_entity->immune) {
			target_entity->vel += pointing * 1.0f + vec3(0,5,0) + vel;
			target_entity->health -= 1;
		}
		//target_entity->alive = false;
		return;
	}
	if (pix == nullptr) {
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
		world->set(pos.x, pos.y, pos.z, blocks->names["backpack"]);
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
	
	if (!consts[4]) {
		nspeed /= 3;
	}
	
	double stamina_cost = 0;
	
	if (!spectator) {
		// Move forward
		
		if (glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS){
			nspeed /= 4;
		}
		
		if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS){
			// vec3 dir_const = vec3(1,0,0)*(float)consts[0] + vec3(0,0,1)*(float)consts[2] + vec3(-1,0,0)*(float)consts[3] + vec3(0,0,-1)*(float)consts[5];
			// if (length(dir_const-forward) < 0.9f and (consts[0] or consts[2] or consts[3] or consts[5]) and autojump and not consts[6]) {
			// 	vel.y = 6;
			// 	if (glfwGetKey( window, GLFW_KEY_SPACE ) == GLFW_PRESS) {
			// 		vel.y = 10;
			// 	}
			// }
			vel += forward * deltaTime * nspeed;
			stamina_cost += glm::dot(vel, forward);
		}
		// Move backward
		if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
			vel -= forward * deltaTime * nspeed;
			stamina_cost += glm::dot(vel, -forward);
		}
		// Strafe right
		if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
				vel += right * deltaTime * nspeed;
				stamina_cost += glm::dot(vel, right);
		}
		// Strafe left
		if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS){
				vel -= right * deltaTime * nspeed;
				stamina_cost += glm::dot(vel, -right);
		}
		if (glfwGetKey( window, GLFW_KEY_SPACE ) == GLFW_PRESS){
			if (consts[4] and vel.y > -0.5) {
				vel.y = 15;// = vec3(0,10,0);//up * deltaTime * speed;
				stamina_cost += vel.y;
			}
		}
		
	} else {
		
		nspeed = 75;
		
		if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS){
			position += forward * deltaTime * nspeed;
		}
		// Move backward
		if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
			position -= forward * deltaTime * nspeed;
		}
		// Strafe right
		if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
			position += right * deltaTime * nspeed;
		}
		// Strafe left
		if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS){
			position -= right * deltaTime * nspeed;
		}
		if (glfwGetKey( window, GLFW_KEY_SPACE ) == GLFW_PRESS){
			position += up * deltaTime * speed;
		}
		if (glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS){
			position -= up * deltaTime * speed;
		}
	}
	
	use_stamina(stamina_cost/30 * deltaTime);
	
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
	// float scale = 0.1f;
	// int i;
	// for (i = 0; i < health; i ++) {
	// 	draw_icon(uivecs, 3, i*scale, 1-scale, scale, scale);
	// }
	// for (; i < 10; i ++) {
	// 	draw_icon(uivecs, 2, i*scale, 1-scale, scale, scale);
	// }
	
	/// draw empty bar
	draw_icon(uivecs, 6, -0.5, 1-0.1*aspect_ratio, 0.1, 0.1*aspect_ratio);
	draw_icon(uivecs, 8, 0.4, 1-0.1*aspect_ratio, 0.1, 0.1*aspect_ratio);
	draw_icon(uivecs, 7, -0.4, 1-0.1*aspect_ratio, 0.8, 0.1*aspect_ratio);
	
	draw_icon(uivecs, 9, -0.45, 1 - 0.1*aspect_ratio, (health - damage_health)/10 * 0.9, 0.1*aspect_ratio);
	if (damage_health > 0) {
		draw_icon(uivecs, 10, (health - damage_health)/10 * 0.9 - 0.45, 1 - 0.1*aspect_ratio, damage_health/10 * 0.9, 0.1*aspect_ratio);
	} else if (healing_health > 0) {
		draw_icon(uivecs, 11, health/10 * 0.9 - 0.45, 1 - 0.1*aspect_ratio, healing_health/10 * 0.9, 0.1*aspect_ratio);
	}
	
	inven.render(uivecs, -0.5f, -1.0f);
	draw_text(uivecs, "\\/", -0.45f+selitem*0.1f, -0.85f);
	Item* sel = inven.get(selitem);
	if (!sel->isnull) {
		draw_text(uivecs, sel->descript(), -0.25f, -0.7f);
	}
}

#endif
