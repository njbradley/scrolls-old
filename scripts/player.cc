#ifndef CONTROLS
#define CONTROLS

#include "entity.h"

#include "blocks.h"
#include "blockiter.h"
#include "items.h"
#include "blockdata.h"
#include "world.h"
#include "menu.h"
#include "blockgroups.h"
#include "ui.h"
#include "cross-platform.h"
#include "text.h"
#include "game.h"
#include "audio.h"
#include "materials.h"
#include "tiles.h"
#include "glue.h"



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
DisplayEntity(newworld, pos, vec3(-0.7,-2.5,-0.7), vec3(0.7,0.9,0.7), new Block(new Pixel(0)), vec3(0,0,0)),
inven(10), backpack(10) { // vec3(-0.8,-3.6,-0.8), vec3(-0.2,0,-0.2)
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
DisplayEntity(newworld, vec3(0,0,0), vec3(-0.7,-2.5,-0.7), vec3(0.7,0.9,0.7), new Block(new Pixel(0)), vec3(0,0,0)),
inven(ifile), backpack(ifile) {
	ifile >> selitem;
	ifile >> position.x >> position.y >> position.z;
	ifile >> vel.x >> vel.y >> vel.z >> angle.x >> angle.y;
	ifile >> health >> max_health >> damage_health >> healing_health >> healing_speed;
	ifile >> flying >> autojump;
	glfwSetMouseButtonCallback(window, mouse_button_call);
	glfwSetScrollCallback(window, scroll_callback);
	update_held_light();
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
	block_breaking_render_index = RenderIndex::npos;
}

void Player::set_block_breaking_vecs(Pixel* pixref, ivec3 hitpos, int level) {/*
	if (level == 0) {
		if (!block_breaking_render_index.isnull()) {
			world->transparent_glvecs.del(block_breaking_render_index);
			block_breaking_render_index = RenderIndex::npos;
		}
		return;
	}
	level --;
	//Pixel pix = *pixref;
	Pixel pix(*pixref);
	pix.render_index = RenderIndex::npos;
	pix.render_flag = true;
	ivec3 gpos = pix.block->globalpos;
	ivec3 pgpos = pix.block->parent->globalpos;
	bool faces[] = {true,true,true,true,true,true};
	MemVecs vecs;
	pix.render(&vecs, &vecs, 0xff, true);
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
	
	if (!block_breaking_render_index.isnull()) {
		world->transparent_glvecs.del(block_breaking_render_index);
	}
	block_breaking_render_index = world->transparent_glvecs.add(&vecs);*/
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

void Player::raycast(Pixel** hit, vec3* hitpos, DisplayEntity** target_entity) {
	vec3 v = position;
	double tiny = 0.00001;
	vec3 pos = position;
	Block* target = world->raycast(&pos, pointing, 8);
	//cout << "ksjdflajsdfklajsd" << endl;
	vector<DisplayEntity*> entities;
	get_nearby_entities(&entities);
	//cout << entities.size() << endl;
	int bx, by, bz;
	float dist;
	if (target != nullptr) {
		dist = glm::length(position-pos);
		*hit = target->pixel;
		*hitpos = pos;
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
	bool shifting = glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS;
	
	if (placing_block == nullptr and placing_freeblock == nullptr) {
		
		vec3 hitpos;
		ivec3 dir;
		Block* block;
		raycast(&block, &dir, &hitpos);
		
		Item* inhand = inven.get(selitem);
		
		if (block != nullptr) {
			if (!inhand->isnull) {
				ivec3 blockpos = block->globalpos + dir;//SAFEFLOOR3(hitpos) + dir;
				if (shifting) {
					world->set_global(blockpos, 1, 0, 0);
					Block* airblock = world->get_global(blockpos.x, blockpos.y, blockpos.z, 2);
					if (airblock != nullptr) {
						cout << "freeblock" << endl;
						// Hitbox newbox (vec3(0,0,0), vec3(blockpos) - 0.5f, vec3(blockpos) + 0.5f);
						Hitbox newbox (vec3(blockpos) + 0.5f, vec3(-0.5f, -0.5f, -0.5f), vec3(0.5f, 0.5f, 0.5f));
						FreeBlock* freeblock = new FreeBlock(newbox);
						freeblock->set_pixel(new Pixel(1));
						airblock->add_freechild(freeblock);
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
					game->debugblock = placing_block->pixel;
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
		return;
		if (placing_freeblock == nullptr and dist > 0.05) {
			cout << "freeblock" << endl;
			Pixel pix = *placing_block->pixel;
			placing_block->pixel->set(0, 0);
			ivec3 blockpos = placing_block->globalpos;
			placing_block = placing_block->get_global(blockpos, 2);
			// Hitbox newbox (vec3(0,0,0), vec3(blockpos) - 0.5f, vec3(blockpos) + 0.5f);
			Hitbox newbox (vec3(blockpos) + 0.5f, vec3(-0.5f, -0.5f, -0.5f), vec3(0.5f, 0.5f, 0.5f));
			FreeBlock* freeblock = new FreeBlock(newbox);
			freeblock->set_pixel(new Pixel(1));
			placing_block->add_freechild(freeblock);
			cout << "freescale " << freeblock->scale << endl;
			//
			placing_freeblock = freeblock;
		}
		if (placing_freeblock != nullptr) {
			quat newrot = glm::angleAxis(dist, vec3(placing_dir));
			//cout << "settting " << placing_freeblock << ' ' << placing_dir << endl;
			Hitbox newbox = placing_freeblock->box;
			// newbox.rotation = newrot;
			// newbox.position += vec3(dist2.x, 0, dist2.y);
			placing_freeblock->move(vec3(0,0,0), newrot * glm::inverse(newbox.rotation));
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
		
		ivec3 pos (hitpos);
		
		world->set_global(pos, 1, 0, 0);
		game->debugblock = world->get_global(pos.x, pos.y-1, pos.z, 1)->pixel;
		cout << game->debugblock->parbl->render_flag << '-' << endl;
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
	if (audio->listener == this) {
		audio->listener = nullptr;
	}
	
	if (menu == nullptr) {
		vector<string> options = {"Respawn"};
		menu = new SelectMenu("You Died", options, [&] (string result) {
			if (result == "Respawn") {
				World* w = world;
				delete world->player;
				w->spawn_player();
			} else if (result == "Quit") {
				cout << "you dead" << endl;
				game->playing = false;
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
		if (camerablock->pixel->value == 7) {
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
			update_held_light();
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
		placing_block = nullptr;
		placing_freeblock = nullptr;
	}
	//mouse = false;
	
	
	
	if (scroll != 0) {
		selitem -= scroll;
		if (selitem < 0) {
			selitem = 0;
		} if (selitem > 9) {
			selitem = 9;
		}
		update_held_light();
		scroll = 0;
	}
	
	if (health <= 0) {
		die();
	}
	
	
	float FoV = settings->initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.
	
	
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

void Player::update_held_light() {
	Item* item = inven.get(selitem);
	if (item->isnull) {
		if (emitted_light != 0) {
			emitted_light = 0;
			emitted_light_flag = true;
		}
	} else {
		if (emitted_light != item->data->lightlevel) {
			emitted_light = item->data->lightlevel;
			emitted_light_flag = true;
		}
	}
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
