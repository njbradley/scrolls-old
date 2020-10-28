#ifndef GAME
#define GAME

#include "game.h"
#include "world.h"
#include "menu.h"
#include "graphics.h"
#include "multithreading.h"
#include "blocks.h"
#include "entity.h"
#include "mobs.h"
#include "blockdata.h"
#include "materials.h"
#include "items.h"
#include "crafting.h"
#include "cross-platform.h"
#include "ui.h"


Game::Game(): graphics(this), threadmanager(this) {
	load_settings();
	min_ms_per_frame = 1000.0/max_fps;
	
	matstorage = new MaterialStorage();
	blocks = new BlockStorage();
	itemstorage = new ItemStorage();
	recipestorage = new RecipeStorage();
	entitystorage = new EntityStorage();
	mobstorage = new MobStorage();
	
	
	ifstream ifile("saves/latest.txt");
	if (!ifile.good()) {
		create_dir("saves");
		ofstream ofile("saves/saves.txt");
		ofile << "";
		world = new World("Starting-World", &threadmanager, 12345);
		graphics.set_world_buffers(world, allocated_memory);
	} else {
		string latest;
		ifile >> latest;
		if (latest != "") {
			world = new World(latest, &threadmanager);
			graphics.set_world_buffers(world, allocated_memory);
		} else {
			ifstream ifile2("saves/saves.txt");
			ifile2 >> latest;
			if (latest != "") {
				world = new World(latest, &threadmanager);
				graphics.set_world_buffers(world, allocated_memory);
			} else {
				world = new World("Starting-World", &threadmanager, 12345);
				graphics.set_world_buffers(world, allocated_memory);
			}
		}
	}
	
	graphics.view_distance = view_dist * World::chunksize * 10;
	
	if (framerate_sync) {
		glfwSwapInterval(1);
	} else {
		glfwSwapInterval(0);
	}
}

Game::~Game() {
	threadmanager.close();
	
	if (world != nullptr) {
		cout << "deleting world" << endl;
		world->close_world();
		delete world;
	}
	
	delete itemstorage;
	delete blocks;
	delete recipestorage;
	delete entitystorage;
	delete mobstorage;
	delete matstorage;
	
	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	if (errors) {
		wait();
	} else {
		cout << "completed without errors" << endl;
	}
}



void Game::setup_gameloop() {
	lastTime = glfwGetTime();
	currentTime = lastTime;
	lastFrameTime = lastTime;
	nbFrames = 0;
	reaching_max_fps = true;
	slow_frame = 0;
	slow_tick = 0;
	graphics.view_distance = view_dist * World::chunksize * 10;
	
	threadmanager.rendering = true;
	audio.listener = world->player;
}


void Game::gametick() {
	
	double begin = glfwGetTime();
	if (menu == nullptr) {
		world->player->computeMatricesFromInputs();
		world->timestep();
		//test->timestep(world);
	}
	double time = glfwGetTime() - begin;
	if (dologtime and time > 0.001) {
		cout << "timestep " << time << endl;
	}
	
	begin = glfwGetTime();
	if (debug_visible) {
		print_debug();
		
	}
	
	world->load_nearby_chunks();
	if (true) {
		//cout << "rendering!!!!" << endl;
		//auto fut =  std::async([&] () {world->render();});//();
		//world->render();
		
		// TileLoop loop(world);
		// for (Tile* tile : loop) {
		// 	if (tile->chunk->render_flag and tile->fully_loaded) {
		// 		tile->render(&world->glvecs, &world->transparent_glvecs);
		// 		world->glvecs.clean();
		//     world->transparent_glvecs.clean();
		//     glFinish();
		// 		break;
		// 	}
		// }
		
		//num_tris = world->glvecs.num_verts/3;//world->glvecs.num_verts/3;
		//make_vertex_buffer(vertexbuffer, uvbuffer, lightbuffer, matbuffer, &num_tris, render_flag);
	}
	
	
	// Measure speed
	lastFrameTime = currentTime;
	currentTime = glfwGetTime();
	//cout << lastFrameTime << ' ' << currentTime << endl;
	double ms = (currentTime-lastFrameTime)*1000;
	if (debug_visible) {
		debugstream << "-----time-----" << endl;
		debugstream << "ms: " << ms << endl;
		debugstream << "ms(goal):" << min_ms_per_frame << endl;
		debugstream << "reaching max fps: " << reaching_max_fps << " (" << slow_frame << ")" << endl;
		debugstream << "tick thread ms: " << threadmanager.tick_ms << endl;
		debugstream << "slow tick: " << slow_tick << endl;
		/// block and entity debug
		
	}
	
	if (ms > min_ms_per_frame) {
		reaching_max_fps = false;
	}
	if (ms > slow_frame) {
		slow_frame = ms;
	}
	if (threadmanager.tick_ms > slow_tick) {
		slow_tick = threadmanager.tick_ms;
	}
	
	nbFrames++;
	if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1 sec ago
			// printf and reset timer
			//printf("%i fps \n", nbFrames);
		fps = nbFrames;
		nbFrames = 0;
			lastTime += 1.0;
		reaching_max_fps = true;
		slow_frame = 0;
		slow_tick = 0;
	}
	
	time = glfwGetTime() - begin;
	if (dologtime and time > 0.001) {
		cout << "debug " << time << endl;
	}
	
	////////////// sleep to max fps
	double sleep_needed = min_ms_per_frame-ms;
	if (!framerate_sync and sleep_needed > 0) {
		
		double time = lastFrameTime + min_ms_per_frame/1000.0;
		double newtime = glfwGetTime();
		std::this_thread::sleep_for(std::chrono::milliseconds(int(min_ms_per_frame-ms)));
	}
	
	double total = glfwGetTime() - lastFrameTime;
	
	if (debug_visible) {
		debugstream << "------ total time -----" << endl;
		debugstream << "total time " << total*1000 << endl;
	}
	currentTime = glfwGetTime();
	
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//-----------------------------------------------------FIRST DRAW CALL-------------------------------------------------------------------------------------------------------------------------//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	double start = glfwGetTime();
	
	world->glvecs.writelock.lock();
	world->transparent_glvecs.writelock.lock();
	time = glfwGetTime() - start;
	if (dologtime and time > 0.001) {
		cout << "unlocking took " << time << endl;
	}
	start = glfwGetTime();
	
	
	graphics.block_draw_call(world);
	
	
	
	world->glvecs.writelock.unlock();
	world->transparent_glvecs.writelock.unlock();
	
	double all = glfwGetTime() - start;
	if (dologtime and all > 0.001) {
		cout << all << " render time" << endl;
	}
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//-----------------------------------------------------SECOND DRAW CALL------------------------------------------------------------------------------------------------------------------------//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	graphics.ui_draw_call(world, &debugstream);
	// Swap buffers
	
	time = glfwGetTime() - begin;
	if (dologtime and time > 0.001) {
		cout << "ui " << time << endl;
	}
	
	begin = glfwGetTime();
	graphics.swap();
	time = glfwGetTime() - begin;
	if (dologtime and time > 0.001) {
		cout << "swap " << time << endl;
	}
	
	audio.tick();
	
	if (menu == nullptr) {
		if (glfwGetKey(window, GLFW_KEY_M ) == GLFW_PRESS) {
			main_menu();
		} else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			inven_menu();
		} else if (glfwGetKey(window, GLFW_KEY_SEMICOLON) == GLFW_PRESS) {
			menu = new CommandMenu(&world->commandprogram, [&]() {
				delete menu;
				menu = nullptr;
			});
		} else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
			menu = new CraftingMenu(1, [&]() {
				delete menu;
				menu = nullptr;
			});
		} else if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
			cout << "njbradley is king" << endl;
		} else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
			world->glvecs.clean_flag = true;
			world->transparent_glvecs.clean_flag = true;
			double s = glfwGetTime();
			world->glvecs.clean();
			world->transparent_glvecs.clean();
			double t = glfwGetTime() - s;
			cout << "time: " << t << endl;
		} else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
			world->player->spectator = true;
		} else if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
			world->player->spectator = false;
		} else if (glfwGetKey(window, GLFW_KEY_O ) == GLFW_PRESS) {
			debug_visible = true;
		} else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
			debug_visible = false;
		} else if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
			double begin = glfwGetTime();
			ivec3 pos(world->player->position);
			for (int i = 0; i < 1000000; i ++) {
				Block* block = world->get_global(pos.x, pos.y, pos.z, 1);
			}
			cout << "1 mil get_global time: " << glfwGetTime() - begin << endl;
		} else if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
			dump_buffers();
			dump_emptys();
			game->crash(1);
		} else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
			map<Material*,char> mat_blocks;
			for (pair<char,BlockData*> kv : blocks->blocks) {
				if (kv.second != nullptr) {
					mat_blocks[kv.second->material] = kv.first;
				}
			}
			ofstream ofile("dig_times.txt");
			vector<char> blockchars;
			ofile << std::setw(30) << ' ';
			vector<string> mats {"wood", "snow", "stone", "bone", "dirt", "grass", "leaves", "sand"};
			for (string name : mats) {
				Material* mat = matstorage->materials[name];
				ofile << std::setw(15) << mat->name;
				blockchars.push_back(blocks->names[name]);
			}
			ofile << endl;
			for (ItemStack& itemstack : world->player->inven.items) {
				if (!itemstack.item.isnull) {
					stringstream name;
					name << itemstack.item.get_name() << " s" << itemstack.item.get_sharpness() << " w" << itemstack.item.get_weight();
					ofile << std::setw(30) << name.str();
					for (char c : blockchars) {
						Pixel pix(0,0,0,c,1,nullptr,nullptr);
						ofile << std::setw(15) << 1.0/itemstack.item.collision(&pix);
					}
					ofile << endl;
				}
			}
			game->crash(443456765);
		}
	} else {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE ) == GLFW_PRESS) {
			menu->close(world);
			delete menu;
			menu = nullptr;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_Q ) == GLFW_PRESS and glfwGetKey(window, 	GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS) {
		//world->closing_world = true;
		playing = false;
	} else if (glfwWindowShouldClose(window)) {
		//world->closing_world = true;
		playing = false;
	}
}

void Game::print_debug() {
	debugstream.str("");
	debugstream << std::fixed;
	debugstream.precision(3);
	
	debugstream << "fps: " << fps << endl;
	debugstream << "x: " << world->player->position.x << "\ny: " << world->player->position.y << "\nz: " << world->player->position.z << endl;
	debugstream << "dx: " << world->player->vel.x << "\ndy: " << world->player->vel.y << "\ndz: " << world->player->vel.z << endl;
	debugstream << "chunk: "
		<< int((world->player->position.x/world->chunksize) - (world->player->position.x<0)) << ' '
		<< int((world->player->position.y/world->chunksize) - (world->player->position.y<0)) << ' '
		<< int((world->player->position.z/world->chunksize) - (world->player->position.z<0)) << endl;
	int loaded_tiles = 0;
	for (Tile* tile : world->tiles) {
		loaded_tiles += (tile != nullptr);
	}
	debugstream << "loaded tiles: " << loaded_tiles << endl;
	debugstream << "mobcount: " << world->mobcount << endl;
	debugstream << "consts: ";
	for (bool b : world->player->consts) {
		debugstream << b << ' ';
	}
	debugstream << endl << "mats: ";
	for (Material* mat : world->player->mat_consts) {
		if (mat == nullptr) {
			debugstream << "air ";
		} else {
			debugstream << mat->name << ' ';
		}
	}
	debugstream << endl;
	world->glvecs.status(debugstream);
	world->transparent_glvecs.status(debugstream);
	audio.status(debugstream);
	debugstream << "------threads-------" << endl;
	debugstream << "load ";
	for (int i = 0; i < threadmanager.num_threads; i ++) {
		//debugstream << "load" << i;
		if (threadmanager.loading[i] != nullptr) {
			ivec3 pos = threadmanager.loading[i]->pos;
			debugstream << pos.x << ',' << pos.y << ',' << pos.z << " ";
		}
	}
	debugstream << endl << "del ";
	for (int i = 0; i < threadmanager.num_threads; i ++) {
		//debugstream << "del " << i;
		if (threadmanager.deleting[i] != nullptr) {
			ivec3 pos = *threadmanager.deleting[i];
			debugstream << pos.x << ',' << pos.y << ',' << pos.z << " ";
		}
	}
	debugstream << endl;
	////////////////////////// error handling:
	debugstream << "-----opengl errors-----" << endl;
	GLenum err;
	while((err = glGetError()) != GL_NO_ERROR) {
		debugstream << "err: " << std::hex << err << std::dec << endl;
		cout << "err: " << std::hex << err << std::dec << endl;
	}
	
	debugstream << "-----block-entity-tracking-----" << endl;
	if (game->debugentity != nullptr) {
		debugstream << "tracking entity " << game->debugentity << endl;
		debugstream << "x:" << game->debugentity->position.x << " y:" << game->debugentity->position.y << " z:" << game->debugentity->position.z << endl;
		debugstream << "dx:" << game->debugentity->vel.x << " dy:" << game->debugentity->vel.y << " dz:" << game->debugentity->vel.z << endl;
		debugstream << "consts: ";
		for (bool b : game->debugentity->consts ) {
			debugstream << b << ' ';
		}
		debugstream << endl;
	}
	if (game->debugblock != nullptr) {
		int gx, gy, gz;
		game->debugblock->global_position(&gx, &gy, &gz);
		string name = "undef";
		BlockData* data = blocks->blocks[game->debugblock->get()];
		if (data != nullptr) {
			name = data->name;
		}
		
		debugstream << "tracking block " << game->debugblock << " at " << gx << "x " << gy << "y " << gz << "z " << endl;
		debugstream << " type:" << name << " char:" << int(game->debugblock->value) << " scale:" << game->debugblock->scale << endl;
		debugstream << " direction:" << int(game->debugblock->direction) << " render_index:" << game->debugblock->render_index.first << ',' << game->debugblock->render_index.second << endl;
		debugstream << " parent coords:" << game->debugblock->px << ',' << game->debugblock->py << ',' << game->debugblock->pz << endl;
		debugstream << " physics_group:" << game->debugblock->physicsgroup << endl;
		if (game->debugblock->physicsgroup != nullptr) {
			//game->debugblock->physicsgroup->to_file(debugstream);
		}
	}
}


void Game::inven_menu() {
	menu = new ToolMenu( [&] () {
		delete menu;
		menu = nullptr;
	});
}

void Game::level_select_menu() {
	vector<string> worlds;
	ifstream ifile("saves/saves.txt");
	string name;
	while (ifile >> name) {
		worlds.push_back(name);
	}
	
	menu = new SelectMenu("Level Select:", worlds, [&] (string result) {
		if (result != "") {
			World* w = world;
			threadmanager.rendering = false;
			world = nullptr;
			w->close_world();
			delete w;
			world = new World(result, &threadmanager);
			graphics.set_world_buffers(world, allocated_memory);
			threadmanager.rendering = true;
			//debug_visible = true;
		}
		delete menu;
		menu = nullptr;
	});
}

void Game::new_world_menu() {
	menu = new TextInputMenu("Enter your new name:", true, [&] (string result) {
		if (result != "") {
			World* w = world;
			threadmanager.rendering = false;
			world = nullptr;
			w->close_world();
			delete w;
			world = new World(result, &threadmanager, time(NULL));
			graphics.set_world_buffers(world, allocated_memory);
			threadmanager.rendering = true;
		}
		delete menu;
		menu = nullptr;
	});
}

void Game::main_menu() {
	vector<string> options = {"Level Select", "New World"};
	menu = new SelectMenu("Scrolls: an adventure game", options, [&] (string result) {
		if (result == "Level Select") {
			delete menu;
			level_select_menu();
		} else if (result == "New World") {
			delete menu;
			new_world_menu();
		} else {
			delete menu;
			menu = nullptr;
		}
	});
}





void Game::crash(long long err_code) {
	if (!errors) {
		cout << "fatal error encountered! shutting down cleanly: err code: " << err_code << endl;
	}
	//dump_buffers();
	//dump_emptys();
	game->errors = true;
	game->playing = false;
}

void Game::hard_crash(long long err_code) {
	cout << "fatal error encountered! unable to close cleanly: err code: " << err_code << endl;
	//dump_buffers();
	//dump_emptys();
	errors = true;
	wait();
	exit(1);
}




void Game::set_display_env(vec3 new_clear_color, int new_view_dist) {
	graphics.clearcolor = new_clear_color;
	graphics.view_distance = new_view_dist;
}

void Game::dump_buffers() {
	cout << "dumping out contents of memory vectors" << endl;
	cout << "dumping vertex data" << endl;
	float* data = (float*)glMapNamedBuffer( world->glvecs.vertexbuffer, GL_READ_ONLY);
	int len = allocated_memory*3;
	ofstream ofile("dump_buffers_verts.txt");
	for (int i = 0; i < allocated_memory*3; i ++) {
		ofile << i/6/3 << ' ' << data[i] << endl;
		// if (data[i] > 100000 or i%(len/100) == 0 or i < 1000) {
		//
		// }
	}
	ofile << "numverts" << world->glvecs.num_verts/6 << endl;
	//char* dat = (char*)data;
	//ofstream ofile("datadump.dat");
	//ofile.write(dat, len*sizeof(float));
	glUnmapNamedBuffer( world->glvecs.vertexbuffer );
	
	cout << "dumping material data" << endl;
	int* matdata = (int*)glMapNamedBuffer( world->glvecs.matbuffer, GL_READ_ONLY);
	len = allocated_memory*2;
	ofstream mofile("dump_buffers_mat.txt");
	for (int i = 0; i < allocated_memory*2; i ++) {
		//if (matdata[i] > 100000 or i%(len/100) == 0 or i < 1000) {
			mofile << i/6/2 << ' ' << matdata[i] << endl;
		//}
	}
	mofile << "numverts" << world->glvecs.num_verts/6 << endl;
	//char* dat = (char*)data;
	//ofstream ofile("datadump.dat");
	//ofile.write(dat, len*sizeof(float));
	glUnmapNamedBuffer( world->glvecs.matbuffer );
	
	cout << "dumping light data" << endl;
	data = (float*)glMapNamedBuffer( world->glvecs.lightbuffer, GL_READ_ONLY);
	len = allocated_memory;
	ofstream fofile("dump_buffers_light.txt");
	for (int i = 0; i < allocated_memory*2; i ++) {
		//if (data[i] > 100000 or i%(len/100) == 0 or i < 1000) {
			fofile << i/6 << ' ' << data[i] << endl;
		//}
	}
	fofile << "numverts" << world->glvecs.num_verts/6 << endl;
	//char* dat = (char*)data;
	//ofstream ofile("datadump.dat");
	//ofile.write(dat, len*sizeof(float));
	glUnmapNamedBuffer( world->glvecs.lightbuffer );
	
}

void Game::dump_emptys() {
	ofstream ofile("dump_emptys.txt");
	for (pair<int,int> index : world->glvecs.empty) {
		ofile << index.first << ' ' << index.second << endl;
	}
}



void Game::load_settings() {
	ifstream ifile("saves/settings.txt");
	if (ifile.good()) {
		string name;
		while (!ifile.eof()) {
			getline(ifile, name, ':');
			if (name == "fov") {
				ifile >> initialFoV;
			} else if (name == "fullscreen") {
				ifile >> name;
				fullscreen = name == "on";
			} else if (name == "max_fps") {
				ifile >> max_fps;
			} else if (name == "framerate_sync") {
				string sync;
				ifile >> sync;
				framerate_sync = sync == "on";
			} else if (name == "view_dist") {
				ifile >> view_dist;
			} else if (name == "alloc_memory") {
				ifile >> allocated_memory;
				allocated_memory *= 6;
			} else if (name == "dims") {
				int x,y;
				ifile >> x >> y;
				set_screen_dims(x, y);
			} else if (name == "") {
				continue;
			} else {
				cout << "error reading settings file" << endl;
				cout << "no setting '" << name << "'" << endl;
				exit(1);
			}
			getline(ifile, name);
		}
	} else {
		create_dir("saves");
		ofstream ofile("saves/settings.txt");
		ofile << "fov: 90" << endl;
		ofile << "fullscreen: true" << endl;
		ofile << "dims: 1600 900" << endl;
		ofile << "max_fps: 90" << endl;
		ofile << "framerate_sync: on" << endl;
		ofile << "alloc_memory: 700000" << endl;
		ofile << "view_dist: 3" << endl;
		load_settings();
	}
}











#endif
