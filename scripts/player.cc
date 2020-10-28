#ifndef CONTROLS
#define CONTROLS

#include "entity.h"

#include "blocks.h"
#include "items.h"
#include "blockdata.h"
#include "world.h"
#include "menu.h"
#include "blockphysics.h"
#include "ui.h"
#include "cross-platform.h"
#include "text.h"
#include "game.h"
#include "audio.h"



/*
// Initial position : on +Z
glm::vec3 position = glm::vec3( 25, 45, 5 );
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
*/


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
	
	init_block_breaking_vecs();
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
		
		init_block_breaking_vecs();
}

void Player::init_block_breaking_vecs() {
	vector<string> files;
	get_files_folder("resources/textures/blocks/transparent/1", &files);
	block_breaking_tex = 0;
	for (int i = 0; i < files.size(); i ++) {
		if (files[i] == "breaking0.png") {
			block_breaking_tex = i;
		}
	}
	block_breaking_render_index = pair<int,int> (-1,0);
}

void Player::set_block_breaking_vecs(Pixel* pixref, ivec3 hitpos, int level) {
	if (level == 0) {
		if (block_breaking_render_index.first != -1) {
			world->transparent_glvecs.del(block_breaking_render_index);
			block_breaking_render_index = pair<int,int>(-1,0);
		}
		return;
	}
	level --;
	int gx, gy, gz;
	Pixel pix = *pixref;
	pix.render_index = pair<int,int>(-1,0);
	pix.render_flag = true;
	pix.global_position(&gx, &gy, &gz);
	int pgx, pgy, pgz;
	pix.parent->global_position(&pgx, &pgy, &pgz);
	bool faces[] = {true,true,true,true,true,true};
	MemVecs vecs;
	pix.render(&vecs, &vecs, world, pgx, pgy, pgz, 1, faces, false, false);
	for (int i = 0; i < vecs.num_verts; i ++) {
		if (vecs.verts[i*3+0] > gx) {
			vecs.verts[i*3+0] = hitpos.x + 1.01;
		} else {
			vecs.verts[i*3+0] = hitpos.x - 0.01;
		}
		
		if (vecs.verts[i*3+1] > gy) {
			vecs.verts[i*3+1] = hitpos.y + 1.01;
		} else {
			vecs.verts[i*3+1] = hitpos.y - 0.01;
		}
		
		if (vecs.verts[i*3+2] > gz) {
			vecs.verts[i*3+2] = hitpos.z + 1.01;
		} else {
			vecs.verts[i*3+2] = hitpos.z - 0.01;
		}
		
		if (vecs.uvs[i*2+0] > 0) {
			vecs.uvs[i*2+0] = 1;
		}
		if (vecs.uvs[i*2+1] > 0) {
			vecs.uvs[i*2+1] = 1;
		}
		
		vecs.mats[i*2+1] = block_breaking_tex + level;
	}
	
	if (block_breaking_render_index.first != -1) {
		world->transparent_glvecs.del(block_breaking_render_index);
	}
	block_breaking_render_index = world->transparent_glvecs.add(&vecs);
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
}

void Player::right_mouse(double deltatime) {
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
		game->debugblock = nullptr;
		game->debugentity = nullptr;
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
	x -= pointing.x*0.002;
	y -= pointing.y*0.002;
	z -= pointing.z*0.002;
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
					game->audio.play_sound("place", vec3(x, y, z));
				}
			}
			inven.use(selitem);
		} else {
			game->debugblock = pix;
			game->debugentity = target_entity;
		}
	
	}
}

void Player::left_mouse(double deltatime) {
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
	// if (pix == nullptr) {
	// 	return;
	// }
	
	
	//cout << "skdfls " << int(pix->value) << endl;
	
	if (attack_recharge <= 0) {
		Item* item = inven.get(selitem);
		
		if (pix != nullptr) {
			
			double x = hitpos.x;
			double y = hitpos.y;
			double z = hitpos.z;
			
			//ivec3 blockpos = ivec3((int)x - (x<0), (int)y - (y<0), (int)z - (z<0));
			
			//cout << hitpos.x << ' ' << hitpos.y << ' ' << hitpos.z << endl;
			int gx,gy,gz;
			pix->global_position(&gx, &gy, &gz);
			//cout << gx << ' ' << gy << ' ' << gz << endl;
			
			ivec3 blockpos = ivec3(x, y, z) - ivec3(x<0, y<0, z<0);
			
			//cout << blockpos.x << ' ' << blockpos.y << ' ' << blockpos.z << endl;
			//cout << ':' << x << ' ' << y << ' ' << z << endl;
			if (blockpos != block_breaking) {
				break_progress = 0;
				block_breaking = blockpos;
			}
			
			if (!item->isnull and item->data->sharpenable and inven.items[selitem].count > 1) {
				inven.make_single(selitem);
			}
			break_progress += item->collision(pix);
			game->audio.play_sound("break", vec3(blockpos));
			//cout << break_progress << endl;
			
			
			if (true and break_progress >= 1) {
				
				set_block_breaking_vecs(pix, blockpos, 0);
				
				game->audio.play_sound("hit", position);
				// double time_needed = item->dig_time(pix->get());
				// double damage_per_sec = 1/time_needed;
				//
				// item->damage(this, blocks->blocks[pix->get()], deltatime);
				char newitem;
				if (item->isnull) {
					newitem = Item::ondig_null(world, blockpos.x, blockpos.y, blockpos.z);
				} else {
					newitem = item->ondig(world, blockpos.x, blockpos.y, blockpos.z);
				}
				if (newitem != 0) {
					blocks->blocks[newitem]->droptable.drop(&inven, this, item);
				}
				break_progress = 0;
			} else {
				set_block_breaking_vecs(pix, blockpos, int(break_progress*7));
			}
		}
		attack_recharge = item->get_recharge_time();
		total_attack_recharge = item->get_recharge_time();
		if (total_attack_recharge <= 0) {
			total_attack_recharge = 0.2;
		}
	}
	//world->mark_render_update(pair<int,int>((int)position.x/world->chunksize - (position.x<0), (int)position.z/world->chunksize - (position.z<0)));
}

void Player::die() {
	if (game->audio.listener == this) {
		game->audio.listener = nullptr;
	}
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
	
	Block* camerablock = world->get_global(int(position.x), int(position.y), int(position.z), 1);
	if (camerablock != 0) {
		if (camerablock->get() == 7) {
			game->set_display_env(vec3(0.2,0.2,0.6), 10);
		} else {
			game->set_display_env(vec3(0.4,0.7,1.0), 2000000);
		}
	}
	
	
	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);
	
	if (attack_recharge > 0) {
		attack_recharge -= deltaTime;
	}
	
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
			if (in_water) {
				vel.y += 3 * deltaTime * nspeed;
			} else if (consts[4] and vel.y > -10) {
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
		left_mouse(deltaTime);
		timeout += deltaTime;
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		right_mouse(deltaTime);
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
	
	
	float FoV = game->initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.
	
	
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
		draw_text(uivecs, sel->descript(), -0.25f, -0.65f);
	}
	sel->render(uivecs, 0.75, -1 - 0.5 * (attack_recharge / total_attack_recharge), 0.25);
}

#endif
