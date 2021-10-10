#include "player.h"

#include "blocks.h"
#include "blockiter.h"
#include "items.h"
#include "blockdata.h"
#include "world.h"
#include "ui.h"
#include "cross-platform.h"
#include "game.h"
#include "audio.h"
#include "materials.h"
#include "tiles.h"
#include "settings.h"
#include "debug.h"

/*
// Initial position : on +Z
glm::vec3 position = glm::vec3( 25, 45, 5 );
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
*/

DEFINE_PLUGIN(Controls);

Plugin<Controls> controls;

float speed = 80.0f; // 3 units / second
float mouseSpeed = 0.003f;



Player::Player(World* newworld, vec3 pos):
Entity(newworld, pos, vec3(-0.7,-2.5,-0.7), vec3(0.7,0.9,0.7)),
inven(10), backpack(10) {
	if (newworld == nullptr) {
		cout << 61 << endl;
	}
	//char arr[] = {1,0,1,0,1,0,1,0}
	selitem = 0;
	
	//inven.add( ItemStack(Item(itemstorage->items["chips"]), 10) );
	//inven.add( ItemStack(Item(itemstorage->items["crafting"]), 1));
}

Player::Player(World* newworld, istream& ifile):
Entity(newworld, vec3(0,0,0), vec3(-0.7,-2.5,-0.7), vec3(0.7,0.9,0.7)),
inven(ifile), backpack(ifile) {
	ifile >> selitem;
	ifile >> position.x >> position.y >> position.z;
	ifile >> vel.x >> vel.y >> vel.z >> angle.x >> angle.y;
	ifile >> health >> max_health >> damage_health >> healing_health >> healing_speed;
	ifile >> spectator >> autojump;
}

void Player::set_block_breaking_vecs(Pixel* pixref, ivec3 hitpos, int level) {
	
}

void Player::save_to_file(ostream& ofile) {
	inven.save_to_file(ofile);
	backpack.save_to_file(ofile);
	ofile << selitem << ' ';
	ofile << position.x << ' ' << position.y << ' ' << position.z << ' ';
	ofile << vel.x << ' ' << vel.y << ' ' << vel.z << ' ' << angle.x << ' ' << angle.y << ' ';
	ofile << health << ' ' << max_health << ' ' << damage_health << ' ' << healing_health << ' ' << healing_speed << ' ';
	ofile << spectator << ' ' << autojump << ' ';
}
	
mat4 Player::getViewMatrix(){
	return ViewMatrix;
}
mat4 Player::getProjectionMatrix(){
	return ProjectionMatrix;
}

void Player::raycast(Block** hit, ivec3* dir, vec3* hitpos) {
	*hitpos = position;
	*hit = world->raycast(hitpos, pointing, 8);
	if (*hit != nullptr) {
		if ((*hit)->freecontainer != nullptr) {
			vec3 pos = (*hit)->freecontainer->box.transform_in(*hitpos);
			vec3 backpos = pos - (*hit)->freecontainer->box.transform_in_dir(pointing) * 0.002f;
			*dir = SAFEFLOOR3(backpos) - SAFEFLOOR3(pos);
			cout << pos << ' ' << backpos << ' ' << *dir << endl;
		} else {
			*dir = SAFEFLOOR3(*hitpos - pointing*0.002f) - SAFEFLOOR3(*hitpos);
		}
	}
}

void Player::right_mouse(double deltatime) {
	//cout << "riht" << endl;
	bool shifting = controls->key_pressed(controls->KEY_SHIFT);
	
	if (placing_block == nullptr and placing_freeblock == nullptr) {
		
		vec3 hitpos;
		ivec3 dir;
		Block* block;
		raycast(&block, &dir, &hitpos);
		
		Item* inhand = inven.get(selitem);
		
		if (block != nullptr) {
			if (true or !inhand->isnull) {
				ivec3 blockpos = block->globalpos + dir;//SAFEFLOOR3(hitpos) + dir;
				if (shifting) {
					world->set_global(blockpos, 1, 0, 0);
					Block* airblock = world->get_global(blockpos.x, blockpos.y, blockpos.z, 2);
					if (airblock != nullptr) {
						cout << "freeblock" << endl;
						// Hitbox newbox (vec3(0,0,0), vec3(blockpos) - 0.5f, vec3(blockpos) + 0.5f);
						// Hitbox newbox (vec3(blockpos) + 0.5f, vec3(-0.5f, -0.5f, -0.5f), vec3(0.5f, 0.5f, 0.5f), glm::angleAxis(1.0f, vec3(dir)));
						Hitbox newbox (vec3(blockpos) + 0.5f, vec3(-0.5f, -0.5f, -0.5f), vec3(0.5f, 0.5f, 0.5f));
						newbox.move(vec3(0,0,0.1));
						// newbox.rotation = glm::angleAxis(3.14159265f/2, vec3(1,0,0));
						// debuglines->render(newbox);
						FreeBlock* freeblock = new FreeBlock(Movingbox(newbox, vec3(0,0,0), vec3(0,0,0), 1));
						// FreeBlock* freeblock = new FreeBlock(Movingbox(newbox, vec3(-0.8,0,0), vec3(0,0,0), 1));
						freeblock->paused = true;
						freeblock->set_pixel(new Pixel(1));
						airblock->add_freechild(freeblock);
						freeblock->calculate_mass();
						// FreeBlock* freeblock = new FreeBlock(vec3(airblock->globalpos), quat(1, 0, 0, 0));
						// freeblock->set_parent(nullptr, airblock->world, ivec3(0,0,0), 1);
						// freeblock->set_pixel(new Pixel(1));
						// airblock->set_freeblock(freeblock);
						//
						placing_freeblock = freeblock;
						placing_block = airblock;
					}
				} else {
					cout << "normal" << endl;
					block->set_global(blockpos, 1, 1, 0);
					placing_block = block->get_global(blockpos, 1);
					// game->debugblock = placing_block->pixel;
					// world->get_global(ox, oy, oz, 1)->set_global(ivec3(ox+dx, oy+dy, oz+dz), 1, 1, 0);
				}
				placing_dir = dir;
				placing_pointing = angle;
			} else {
				cout << block << '.' << block->freecontainer << endl;
				placing_block = block;
				placing_freeblock = block->freecontainer;
				placing_dir = dir;
				placing_pointing = angle;
			}
		}
	} else {
		float dist = (placing_pointing.y - angle.y) * 6.0f;
		vec2 dist2 = (placing_pointing - angle) * 6.0f;
		
		if (false and placing_freeblock == nullptr and dist > 0.05) {
			cout << "freeblock" << endl;
			Pixel pix = *placing_block->pixel;
			placing_block->pixel->set(0, 0);
			ivec3 blockpos = placing_block->globalpos;
			placing_block = placing_block->get_global(blockpos, 2);
			// Hitbox newbox (vec3(0,0,0), vec3(blockpos) - 0.5f, vec3(blockpos) + 0.5f);
			Hitbox newbox (vec3(blockpos) + 0.5f, vec3(-0.5f, -0.5f, -0.5f), vec3(0.5f, 0.5f, 0.5f));
			FreeBlock* freeblock = new FreeBlock(Movingbox(newbox, 1));
			freeblock->set_pixel(new Pixel(1));
			placing_block->add_freechild(freeblock);
			freeblock->calculate_mass();
			cout << "freescale " << freeblock->scale << endl;
			//
			placing_freeblock = freeblock;
		}
		if (false and placing_freeblock != nullptr) {
			quat newrot = glm::angleAxis(dist, vec3(placing_dir));
			//cout << "settting " << placing_freeblock << ' ' << placing_dir << endl;
			Hitbox newbox = placing_freeblock->box;
			newbox.rotation = newrot;
			placing_freeblock->set_box(newbox);
			// newbox.position += vec3(dist2.x, 0, dist2.y);
			// placing_freeblock->move(vec3(0,0,0), newrot * glm::inverse(newbox.rotation));
			//placing_freeblock->set_rotation(newrot);
			// placing_block->set_render_flag();
		}
	}
	return;
	/*
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
	
	ivec3 blockpos = ivec3(hitpos) - ivec3(hitpos.x<0, hitpos.y<0, hitpos.z<0);
	ivec3 dir (dx, dy, dz);
	int closest_side = -1;
	vec3 inblock = hitpos - glm::floor(hitpos);
	vec3 absinblock = glm::abs(inblock - 0.5f);
	float side_dist = 0.5f;
	for (int i = 0; i < 3; i ++) {
		if (dir[i] == 0 and (closest_side == -1 or absinblock[i] > absinblock[closest_side])) {
			closest_side = i;
		}
	}
	ivec3 close_dir (0,0,0);
	if (inblock[closest_side] - 0.5f < 0) {
		close_dir[closest_side] = -1;
	} else {
		close_dir[closest_side] = 1;
	}
	side_dist -= absinblock[closest_side];
	
	int close_index = -1;
	for (int i = 0; i < 6; i ++) {
		if (close_dir == dir_array[i]) {
			close_index = i;
		}
	}
	ivec3 cross_dir = ivec3(glm::cross(vec3(dir), vec3(close_dir)));
	
	
	if (blocks->blocks[pix->value]->rcaction != "null" and !shifting) {
		blocks->blocks[pix->value]->do_rcaction(pix);
	} else {
		if (!inhand->isnull and inhand->data->onplace != nullptr) {
			
			
			
			return;
			Item* item = inven.get(selitem);
			if (!item->isnull) {
				CharArray* arr = item->data->onplace;
				if (arr != nullptr) {
					
					ivec3 dirs[] = {
						dir,
						close_dir,
						-close_dir,
						cross_dir,
						-cross_dir
					};
					
					if (shifting and side_dist < 0.3 and world->get(blockpos.x+close_dir.x, blockpos.y+close_dir.y, blockpos.z+close_dir.z) == 0) {
						dirs[0] = close_dir;
						dirs[1] = -close_dir;
						dirs[2] = dir;
					}
					
					for (ivec3 newdir : dirs) {
						bool placed = arr->place(world, item, blockpos+dir, newdir);
						if (placed) {
							audio->play_sound("place", vec3(x, y, z));
							inven.use(selitem);
							break;
						}
					}
				}
			}
		} else if (!inhand->isnull and inhand->data->glue != nullptr) {
			
			// cout << close_dir << ' '<< close_index << endl;
			if (close_index != -1 and pix->joints[close_index] == 7) {
				pix->joints[close_index] = inhand->data->glue->tex_index;
				ivec3 sidepos = blockpos + close_dir;
				Pixel* sidepix = world->get_global(sidepos.x, sidepos.y, sidepos.z, 1)->pixel;
				sidepix->joints[(close_index+3)%6] = inhand->data->glue->tex_index;
				pix->render_update();
				sidepix->render_update();
				if (pix->group != nullptr) pix->group->del(pix);
				if (sidepix->group != nullptr) sidepix->group->del(pix);
				world->block_update(blockpos.x, blockpos.y, blockpos.z);
				world->block_update(sidepos.x, sidepos.y, sidepos.z);
			}
		} else {
			game->debugblock = pix;
			game->debugentity = target_entity;
		}
	
	}*/
}

void Player::left_mouse(double deltatime) {
	
	if (timeout <= 0) {
		
		vec3 hitpos;
		ivec3 dir;
		Block* block;
		raycast(&block, &dir, &hitpos);
		
		ivec3 pos = SAFEFLOOR3(hitpos);
		
		if (block->freecontainer == nullptr) {
			world->set_global(pos, 1, 0, 0);
		} else {
			block->freecontainer->apply_impulse(pointing, hitpos);
		}
		// game->debugblock = world->get_global(pos.x, pos.y-1, pos.z, 1)->pixel;
		// cout << game->debugblock->parbl->flags << '-' << endl;
		timeout = 0.5;
	}
	/*
	
	
	vec3 hitpos;
	Pixel* pix;
	DisplayEntity* target_entity;
	raycast(&pix, &hitpos, &target_entity);
	
	if (attack_recharge <= 0) {
		Item* item = inven.get(selitem);
		bool shifting = glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS;
		
		if (target_entity != nullptr) {
			if (!target_entity->immune) {
				target_entity->vel += pointing * 1.0f + vec3(0,5,0) + vel;
				if (item->isnull) {
					target_entity->damage(1);
				} else {
					target_entity->damage(item->get_sharpness());
				}
			}
			} else if (pix != nullptr) {
				ivec3 blockpos = ivec3(hitpos) - ivec3(hitpos.x<0, hitpos.y<0, hitpos.z<0);
			if (shifting and pix->group != nullptr and pix->group->isolated and pix->group->size < 20) {
				Item newitem = itemstorage->from_group(pix);
				if (!newitem.isnull) {
					inven.add(ItemStack(newitem, 1));
					for (Pixel* pix : pix->iter_group()) {
						pix->set(0);
					}
				}
			} else if (shifting and pix->group == nullptr) {
				world->block_update(blockpos.x, blockpos.y, blockpos.z);
				world->aftertick([&, pix] (World* world) {
					if (pix->group != nullptr and pix->group->isolated and pix->group->size < 20) {
						Item newitem = itemstorage->from_group(pix);
						if (!newitem.isnull) {
							inven.add(ItemStack(newitem, 1));
							for (Pixel* pix : pix->iter_group()) {
								pix->set(0);
							}
						}
					}
				});
			} else if (!shifting) {
				
				vec3 shifted_hitpos = hitpos - pointing * 0.002f;
				ivec3 main_dir = ivec3(shifted_hitpos) - ivec3(shifted_hitpos.x<0, shifted_hitpos.y<0, shifted_hitpos.z<0) - blockpos;
				
				int closest_side = -1;
				vec3 inblock = hitpos - glm::floor(hitpos);
				vec3 absinblock = glm::abs(inblock - 0.5f);
				for (int i = 0; i < 3; i ++) {
					if (main_dir[i] == 0 and (closest_side == -1 or absinblock[i] > absinblock[closest_side])) {
						closest_side = i;
					}
				}
				ivec3 close_dir (0,0,0);
				float side_dist = 0.5f - absinblock[closest_side];
				if (inblock[closest_side] - 0.5f < 0) {
					close_dir[closest_side] = -1;
				} else {
					close_dir[closest_side] = 1;
				}
				
				ivec3 cross_dir = ivec3(glm::cross(vec3(main_dir), vec3(close_dir)));
				
				ivec3 chosen_pos = blockpos;
				ivec3 chosen_dir = close_dir;
				int chosen_dir_index = -1;
				
				// close_dir: dir that the hit of impact is closest to
				// main_dir: dir towards the air of the hit block
				// cross_dir: cross of close and main
				ivec3 options[][2] = {
					{blockpos, close_dir},
					{blockpos, cross_dir},
					// {blockpos + cross_dir + main_dir, close_dir},
					// {blockpos - cross_dir + main_dir, close_dir},
					// {blockpos + cross_dir, close_dir},
					// {blockpos - cross_dir, close_dir},
					{blockpos, -cross_dir},
					{blockpos, -close_dir},
					{blockpos, -main_dir}
				};
				
				if (side_dist > 0.3f) {
					for (int i = 3; i >= 0; i --) {
						options[i+1][0] = options[i][0];
						options[i+1][1] = options[i][1];
					}
					options[0][0] = blockpos;
					options[0][1] = -main_dir;
				}
				
				for (int i = 0; i < 5; i ++) {
					ivec3 pos = options[i][0];
					ivec3 dir = options[i][1];
					int dir_index;
					for (int i = 0; i < 6; i ++) {
						if (dir_array[i] == dir) dir_index = i;
					}
					
					Pixel* pix = world->get_global(pos.x, pos.y, pos.z, 1)->pixel;
					if (pix->value != 0) {
						Pixel* otherpix = world->get_global(pos.x+dir.x, pos.y+dir.y, pos.z+dir.z, 1)->pixel;
						if (blocks->clumpy(otherpix->value, pix->value)) {
							if (pix->parbl->scale != 1 or otherpix->parbl->scale != 1) {
								world->set(ivec4(pos.x, pos.y, pos.z, 1), -1, 0);
								world->set(ivec4(pos.x+dir.x, pos.y+dir.y, pos.z+dir.z, 1), -1, 0);
								chosen_pos = pos;
								chosen_dir = dir;
								chosen_dir_index = dir_index;
								break;
							} else if (pix->joints[dir_index] != 7) {
								chosen_pos = pos;
								chosen_dir = dir;
								chosen_dir_index = dir_index;
								break;
							}
						}
					}
				}
				
				if (chosen_dir_index != -1) {
					ivec3 chosen_opos = chosen_pos + chosen_dir;
					Pixel* pix = world->get_global(chosen_pos.x, chosen_pos.y, chosen_pos.z, 1)->pixel;
					Pixel* otherpix = world->get_global(chosen_opos.x, chosen_opos.y, chosen_opos.z, 1)->pixel;
					
					pix->joints[chosen_dir_index] = 7;
					otherpix->joints[(chosen_dir_index+3)%6] = 7;
					pix->render_update();
					otherpix->render_update();
					
					if (pix->joints[chosen_dir_index] == 7) {
						ivec3 pos = pix->parbl->globalpos;
						if (pix->group != nullptr) pix->group->del(pix);
						pix->parbl->world->block_update(pos);
					}
					if (otherpix->joints[(chosen_dir_index+3)%6] == 7) {
						ivec3 pos = otherpix->parbl->globalpos;
						if (otherpix->group != nullptr) otherpix->group->del(otherpix);
						otherpix->parbl->world->block_update(pos);
					}
				}
			}
		}
		
		attack_recharge = item->get_recharge_time();
		total_attack_recharge = item->get_recharge_time();
		if (total_attack_recharge <= 0) {
			total_attack_recharge = 0.2;
		}
	}*/
}

void Player::die() {
	audio->set_listener(nullptr, nullptr, nullptr);
	
	// if (menu == nullptr) {
	// 	vector<string> options = {"Respawn"};
	// 	menu = new SelectMenu("You Died", options, [&] (string result) {
	// 		if (result == "Respawn") {
	// 			World* w = world;
	// 			delete world->player;
	// 			w->spawn_player();
	// 		} else if (result == "Quit") {
	// 			cout << "you dead" << endl;
	// 			game->playing = false;
	// 		}
	// 		delete menu;
	// 		menu = nullptr;
	// 	});
	// }
}

void Player::computeMatricesFromInputs(){

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = getTime();
	
	Block* camerablock = world->get_global(int(position.x), int(position.y), int(position.z), 1);
	if (camerablock != 0) {
		// if (camerablock->pixel->value == 7) {
		// 	game->set_display_env(vec3(0.2,0.2,0.6), 10);
		// } else {
		// 	game->set_display_env(vec3(0.4,0.7,1.0), 2000000);
		// }
	}
	
	
	// Compute time difference between current and last frame
	double currentTime = getTime();
	float deltaTime = float(currentTime - lastTime);
	
	if (attack_recharge > 0) {
		attack_recharge -= deltaTime;
	}
	
	
	// Compute new orientation
	angle += vec2(float(mouseSpeed) * vec2(settings->screen_dims/2 - controls->mouse_pos()));
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
	
	// Reset mouse position for next frame
	controls->mouse_pos(settings->screen_dims/2);
	
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
	
	if (controls->key_pressed('F')) {
		spectator = true;
	} else if (controls->key_pressed('G')) {
		spectator = false;
	}
	
	if (!controls->key_pressed(controls->KEY_CTRL)) {
		if (!spectator) {
			// Move forward
			
			if (controls->key_pressed(controls->KEY_SHIFT)) {
				nspeed /= 4;
			}
			
			if (controls->key_pressed('W')){
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
			if (controls->key_pressed('S')){
				vel -= forward * deltaTime * nspeed;
				stamina_cost += glm::dot(vel, -forward);
			}
			// Strafe right
			if (controls->key_pressed('D')){
					vel += right * deltaTime * nspeed;
					stamina_cost += glm::dot(vel, right);
			}
			// Strafe left
			if (controls->key_pressed('A')){
					vel -= right * deltaTime * nspeed;
					stamina_cost += glm::dot(vel, -right);
			}
			if (controls->key_pressed(' ')){
				if (in_water) {
					vel.y += 3 * deltaTime * nspeed;
				} else if (consts[4] and vel.y > -10) {
					vel.y = 15;// = vec3(0,10,0);//up * deltaTime * speed;
					stamina_cost += vel.y;
				}
			}
			
		} else {
			
			nspeed = 75;
			
			if (controls->key_pressed('W')){
				position += forward * deltaTime * nspeed;
			}
			// Move backward
			if (controls->key_pressed('S')){
				position -= forward * deltaTime * nspeed;
			}
			// Strafe right
			if (controls->key_pressed('D')){
				position += right * deltaTime * nspeed;
			}
			// Strafe left
			if (controls->key_pressed('A')){
				position -= right * deltaTime * nspeed;
			}
			if (controls->key_pressed(' ')){
				position += up * deltaTime * speed;
			}
			if (controls->key_pressed(controls->KEY_SHIFT)){
				position -= up * deltaTime * speed;
			}
		}
	}
	
	use_stamina(stamina_cost/30 * deltaTime);
	
	for (int i = 0; i < 10; i ++) {
		if (controls->key_pressed((i != 9) ? '1' + i : '0')) {
			selitem = i;
			break;
		}
	}
	
	if (controls->mouse_button(0)) {
		left_mouse(deltaTime);
		timeout += deltaTime;
	}
	else if (controls->mouse_button(1)) {
		right_mouse(deltaTime);
		timeout += deltaTime;
	} else {
		timeout = 0;
		placing_block = nullptr;
		placing_freeblock = nullptr;
	}
	//mouse = false;
	
	if (health <= 0) {
		die();
	}
	
	
	float FoV = settings->fov;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.
	
	
	

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}

void Player::render_ui(UIVecs* uivecs) {
	
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
	// uivecs->add(UIVecs("
	//
	static float aspect_ratio = settings->aspect_ratio();
	
	uivecs->add(UIRect("healthbar-left.png", vec2(-0.5, 1-0.1*aspect_ratio), vec2(0.1, 0.1*aspect_ratio)));
	uivecs->add(UIRect("healthbar-right.png", vec2(0.4, 1-0.1*aspect_ratio), vec2(0.1, 0.1*aspect_ratio)));
	uivecs->add(UIRect("healthbar-mid.png", vec2(-0.4, 1-0.1*aspect_ratio), vec2(0.8, 0.1*aspect_ratio)));
	
	uivecs->add(UIRect("healthbar-red.png",
		vec2(-0.45, 1 - 0.1*aspect_ratio),
		vec2((health - damage_health)/10 * 0.9, 0.1*aspect_ratio)
	));
	if (damage_health > 0) {
		uivecs->add(UIRect("healthbar-white.png",
			vec2((health - damage_health)/10 * 0.9 - 0.45, 1 - 0.1*aspect_ratio),
			vec2(damage_health/10 * 0.9, 0.1*aspect_ratio)
		));
	}
	//
	inven.render(uivecs, vec2(-0.5f, -1.0f));
	
	uivecs->add(UIChar('X', vec2(-0.01f, -0.02f), 0.02f));
	// draw_text(uivecs, "\\/", -0.45f+selitem*0.1f, -0.85f);
	// Item* sel = inven.get(selitem);
	// if (!sel->isnull) {
	// 	draw_text(uivecs, sel->descript(), -0.25f, -0.65f);
	// }
	// sel->render(uivecs, 0.75, -1 - 0.5 * (attack_recharge / total_attack_recharge), 0.25);
}
