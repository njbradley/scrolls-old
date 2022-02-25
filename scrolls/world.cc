#ifndef WORLD
#define WORLD

#include "cross-platform.h"
#include "rendervecs.h"
#include "blockdata.h"
#include "blocks.h"
#include "world.h"
#include "tiles.h"
#include "entity.h"
#include "game.h"
#include "terrain.h"
#include "player.h"
#include "graphics.h"
#include "audio.h"
#include "debug.h"
#include "settings.h"
#include "blockiter.h"

#include <map>


DEFINE_AND_EXPORT_PLUGIN(World);



float World::tick_deltatime = 0.030f;
int World::chunksize = 16;

World::World(string oldname): seed(std::hash<string>()(name)), terrainloader(seed), tileloader(this), name(oldname), physics(this, tick_deltatime) {
  
}

World::~World() {
  close_world();
}

string World::path(string filename) const {
  return SAVES_PATH + name + '/' + filename;
}


void World::parse_config_line(string buff, istream& ifile) {
  if (buff == "seed") {
    ifile >> seed;
    terrainloader.set_seed(seed);
  }
  if (buff == "daytime") {
    ifile >> daytime;
  }
  if (buff == "difficulty") {
    ifile >> difficulty;
  }
  if (buff == "generation") {
    string gen;
    ifile >> gen;
    generation = gen == "on";
  }
  if (buff == "last_player_pos") {
    ifile >> last_player_pos.x >> last_player_pos.y >> last_player_pos.z;
  }
}

void World::load_config(istream& ifile) {
  string buff;
  getline(ifile, buff);
  getline(ifile,buff,':');
  while ( !ifile.eof() and buff != "end" ) {
    parse_config_line(buff, ifile);
    getline(ifile, buff);
    getline(ifile,buff,':');
  }
}

void World::save_config(ostream& ofile) {
  ofile << "Scrolls data file of world '" + name + "'\n";
  ofile << "seed:" << seed << endl;
  ofile << "difficulty:" << difficulty << endl;
  ofile << "daytime:" << daytime << endl;
  ofile << "generation:" << (generation ? "on" : "off") << endl;
  ofile << "last_player_pos:" << last_player_pos.x << ' ' << last_player_pos.y << ' ' << last_player_pos.z << endl;
  ofile << "end:" << endl;
}

void World::setup_files() {
    create_dir(path());
    create_dir(path("chunks"));
    create_dir(path("groups"));
    ofstream ofile(SAVES_PATH "saves.txt", std::ios::app);
    ofile << ' ' << name;
}

void World::startup() {
  ifstream ifile(path("worlddata.txt"));
  if (ifile.good()) {
    load_config(ifile);
  } else {
    setup_files();
  }
  
  tileloader->begin_serving();
  tickrunner.init(this, tick_deltatime);
  
  {
    ifstream ifile(path("player.txt"));
    if (ifile.good()) {
      // player = new Player(this, ifile);
      // set_player_vars();
    } else {
      last_player_pos = ivec3(32,terrainloader.get_height(ivec2(32,32))+4,32);
      last_player_pos = SAFEDIV(last_player_pos, chunksize);
      // spawn_player();
    }
  }
}

void World::spawn_player() {
  cout << "spawning player " << endl;
  ivec3 spawnpos(32,terrainloader.get_height(ivec2(32,32))+8,32);
  // ivec3 spawnpos(32, 37, 32);
  int i = 0;
  // while (loader.gen_func(ivec4(spawnpos.x, spawnpos.y, spawnpos.z, 1)) != 0 and i < 100) {
  //   i ++;
  //   spawnpos = ivec3(spawnpos.x+1,loader.get_height(ivec2(10,10))+4,10);
  // }
  Block* spawnblock = get_global(spawnpos, 8);
  if (spawnblock == nullptr) {
    cout << "player spawn location is null (waiting, though last_player_pos might be corrupt)" << endl;
    return;
  }
  if (!spawnblock->continues and !spawnblock->pixel->done_generating) {
    return;
  }
  player = new Player();
  while (spawnblock->scale > 8) {
    spawnblock->subdivide();
    spawnblock = spawnblock->get_global(spawnpos, 8);
  }
  spawnblock->add_freechild(player);
  player->set_position(spawnpos);
  // player->spectator = true;
  set_player_vars();
}

void World::set_player_vars() {
  audio->set_listener(&player->renderbox.position, &player->box.velocity, &player->angle);
  graphics->set_camera(&player->renderbox.position, &player->angle);
  viewbox->playerpos = &player->renderbox.position;
  player->controller = controls;
  spectator.controller = nullptr;
}

void World::set_spectator_vars() {
  // audio->set_listener(&player->renderbox.position, &player->box.velocity, &player->angle);
  graphics->set_camera(&spectator.position, &spectator.angle);
  viewbox->playerpos = &player->renderbox.position;
  spectator.controller = controls;
  player->controller = nullptr;
}

void World::divide_terrain(Block* block, int max_divides) {
  std::lock_guard<Block> guard(*block);
  
  bool split = false;
  Blocktype val = terrainloader.terrain->gen_block(block->globalpos, block->scale, &split);
  block->set_pixel(new Pixel(val));
  
  if (split) {
    block->set_flag(Block::GENERATION_FLAG);
  } else {
    block->pixel->done_generating = true;
  }
}

BlockIterator<Block> worlditer = BlockIterator<Block>(nullptr);


void World::join_level(Block* block, int level) {
  if (block == nullptr) return;
  
  if (block->continues) {
    if (block->scale == level*2) {
      block->join();
      block->set_pixel(new Pixel(block->prevvalue));
    } else {
      for (int i = 0; i < csize3; i ++) {
        join_level(block->children[i], level);
      }
    }
  }
}

static int num_blocks = 0;

void World::divide_level(Block* block, int level) {
  if (block == nullptr or !(block->flags & Block::GENERATION_FLAG)) return;
  block->reset_flag(Block::GENERATION_FLAG);
  
  if (block->scale == level) {
    if (!block->continues) {
      Blocktype vals[csize3];
      bool splits[csize3];
      bool same_vals = true;
      for (int i = 0; i < 8; i ++) {
        ivec3 newglobalpos = block->globalpos + block->posof(i) * (block->scale/2);
        vals[i] = terrainloader.terrain->gen_block(newglobalpos, block->scale/2, &splits[i]);
        same_vals = same_vals and !splits[i] and vals[i] == vals[0];
      }
      if (!same_vals) {
        block->divide();
        for (int i = 0; i < 8; i ++) {
          Block* newblock = new Block();
          num_blocks ++;
          block->set_child(i, newblock);
          newblock->set_pixel(new Pixel(vals[i]));
          if (splits[i]) {
            newblock->set_flag(Block::GENERATION_FLAG);
          } else {
            newblock->pixel->done_generating = true;
          }
        }
      }
    }
  } else if (block->continues) {
    for (int i = 0; i < csize3; i ++) {
      divide_level(block->children[i], level);
    }
  }
}

void World::load_nearby_chunks() {
  static int max_dist = 1;
  static double startTime = getTime();
  static bool full_gen = false;
  // cout << " last pos " << last_player_pos << endl;
  if (mainblock == nullptr) {
    mainblock = new Block();
    max_scale = 4096;// * World::chunksize;
    mainblock->set_parent(this, ivec3(-max_scale/2), max_scale);
    // mainblock->set_flag(Block::GENERATION_FLAG);
    divide_terrain(mainblock, 4);
    cout << (mainblock->flags & Block::GENERATION_FLAG) << endl;
  } else {
    if (player != nullptr) {
      last_player_pos = SAFEDIV(player->highparent->globalpos, World::chunksize);
    }
    
    
    // worlditer = BlockIterator<Block>(mainblock);
    
    
    // cout << " STARTING ____  -- - - _  __ _ _ _ __ ___-- - _  _" << (mainblock->flags & Block::GENERATION_FLAG) << endl;
    bool changed = false;
    int num = 0;
    for (Block* block : BlockIterable<ChunkIterator<Block>>(mainblock)) { num ++;
      if (block->chunk_scale() != block->scale) {
        cout << block->globalpos << ' ' << block->scale << ' ' << block->chunk_scale() << endl;
      }
      int max_chunk_scale = block->scale;
      int min_chunk_scale = block->chunk_scale() / World::chunksize;
      // cout << block << ' ' << block->globalpos << ' ' << block->scale << ' ' << (block->flags & Block::GENERATION_FLAG) << endl;
      ivec3 blockpos = block->globalpos + block->scale/2;
      blockpos = SAFEDIV(blockpos, World::chunksize);
      float dist = glm::length(vec3(last_player_pos - blockpos));
      dist = std::max(dist - min_chunk_scale/2, 0.0f);
      // cout << dist << endl;
      if (dist >= max_dist) {
        continue;
      }
      
      dist /= 5;
      
      int goal_scale = 1;
      while (goal_scale < dist) {
        goal_scale *= csize;
      }
      
      goal_scale = std::min(std::max(goal_scale, min_chunk_scale), max_chunk_scale);
      // cout << " goal scale " << goal_scale << endl;
      
      while (std::min(std::max(block->min_scale, min_chunk_scale), max_chunk_scale) > goal_scale and (block->flags & Block::GENERATION_FLAG)) {
        // cout << " dividing level " << block->min_scale << endl;
        divide_level(block, std::min(std::max(block->min_scale, min_chunk_scale), max_chunk_scale));
        changed = true;
      }
      
      while (std::min(std::max(block->min_scale, min_chunk_scale), max_chunk_scale) < goal_scale) {
        // cout << " joining level " << block->min_scale << endl;
        join_level(block, std::min(std::max(block->min_scale, min_chunk_scale), max_chunk_scale));
        changed = true;
      }
    }
    
    if (changed) {
      cout << " changed " << last_player_pos << ' ' << max_dist << ' ' << num_blocks << endl;
    }
    if (max_dist >= 3) {
      if (initial_generation) {
        cout << "initial generation done " << endl;
      }
      initial_generation = false;
    }
    
    if (max_dist >= max_scale / chunksize and !full_gen) {
      cout << "Full generation done: " << getTime() - startTime << endl;
      full_gen = true;
    }
    
    if (!changed and max_dist < max_scale / chunksize) {
      max_dist *= 2;
    }
  }
}

void World::timestep(float curtime, float deltatime) {
  
  debuglines->clear("timestep");
  // ivec3 chunk(SAFEDIV(player->box.position, (float)chunksize);
  // //player->position/float(chunksize) - vec3(player->position.x<0, player->position.y<0, player->position.z<0));
  // if (player != nullptr and (tileat(chunk) != nullptr or player->spectator)) {
  //   player->timestep();
  //   player->computeMatricesFromInputs();
  // }
  
  for (FreeBlock* free = allfreeblocks; free != nullptr; free = free->allfreeblocks) {
    if (player == nullptr) {
      player = dynamic_cast<Player*>(free);
      if (player != nullptr) {
        cout << "got player from world" << endl;
        set_player_vars();
      }
    }
    free->timestep(curtime, deltatime);
  }
  
  if (player == nullptr and get_global(last_player_pos * chunksize, 32) != nullptr) {
    spawn_player();
  }
  
  if (controls->key_pressed('F')) {
    spectator.position = player->box.position;
    spectator.angle = player->angle;
    set_spectator_vars();
  } else if (controls->key_pressed('G')) {
    set_player_vars();
  }
  
  spectator.timestep(curtime, deltatime);
  
  daytime += deltatime;
  if (daytime > 2000) {
    daytime -= 2000;
  }
  if (daytime < 0) {
    daytime += 2000;
  }
  
}

void World::tick(float curtime, float deltatime) {
  static bool n_pressed = false;
  static bool paused = true;
  // return;
  if (controls != nullptr) {
    if (controls->key_pressed('M')) {
      paused = false;
    }
    if (controls->key_pressed('B')) {
      paused = true;
    }
    
    bool cur_n_pressed = controls->key_pressed('N');
    if (paused and (!cur_n_pressed or n_pressed)) {
      n_pressed = cur_n_pressed;
      return;
    }
    n_pressed = cur_n_pressed;
  }
  
  debuglines->clear("tick");
  
  for (FreeBlock* free = allfreeblocks; free != nullptr; free = free->allfreeblocks) {
    free->tick(curtime, deltatime);
  }
  
  physics->tick(curtime, deltatime);
}

bool World::render() {
  // static bool start_render = true;
  // if (block == nullptr or initial_generation) return false;
  // if (start_render) {
  //   cout << "starting render" << endl;
  //   start_render = false;
  // }
  load_nearby_chunks();
  bool changed = mainblock->flags & Block::RENDER_FLAG;
  mainblock->render(graphics->blockvecs, graphics->transvecs);
  first_render = false;
  return changed;
}

void World::set_root(Block* nblock) {
  if (mainblock != nullptr) delete mainblock;
  mainblock = nblock;
  mainblock->set_parent(this, ivec3(0,0,0), max_scale);
}

Block* World::get_global(ivec3 pos, int scale) {
  if (mainblock == nullptr) return nullptr;
  return mainblock->get_global(pos, scale);
}

const Block* World::get_global(ivec3 pos, int scale) const {
  if (mainblock == nullptr) return nullptr;
  return mainblock->get_global(pos, scale);
}

void World::set_global(ivec3 pos, int w, Blocktype val, int direction, int joints[6]) {
  mainblock->set_global(pos, w, val, direction, joints);
}

Blocktype World::get(ivec3 pos) {
  Block* block = get_global(pos, 1);
  if (block == nullptr) {
    return -1;
  }
  return block->pixel->value;
}

void World::set(ivec3 pos, Blocktype val, int direction) {
  set_global(pos, 1, val, direction);
}

Block* World::raycast(vec3* pos, vec3 dir, double time) {
  Block* b = get_global(SAFEFLOOR3(*pos), 1);
  // Block* b = get_global((int)pos->x - (pos->x<0), (int)pos->y - (pos->y<0), (int)pos->z - (pos->z<0), 1);
  if (b == nullptr) {
      return b;
  }
  return b->raycast(pos, dir, time);
}

bool World::is_world_closed() {
  return world_closing and mainblock == nullptr and deleting_chunks.size() == 0;
}

void World::close_world() {
  tileloader->end_serving();
  tickrunner.close();
  
  if (saving) {
    ofstream ofile(path("player.txt"));
    ofile << ',' << endl;
    // player->save_to_file(ofile);
    ofile.close();
  }
  
  if (saving) {
    cout << "all tiles saved sucessfully: " << endl;
  } else {
    cout << "no tiles saved due to user command: " << endl;
  }
  
  // for (Tile* tile : tiles) {
  //   tile->deleting = true;
  //   if (saving) {
  //     tile->save();
  //   }
  //   delete tile;
  // }
  ofstream blockfile (path("chunks/chunk.dat"));
  mainblock->to_file(blockfile);
  
  if (saving) {
    ofstream datafile(path("worlddata.txt"));
    save_config(datafile);
  }
  ofstream ofile2(SAVES_PATH "latest.txt");
  ofile2 << name;
}

void World::add_freeblock(FreeBlock* freeblock) {
  freeblock->allfreeblocks = allfreeblocks;
  allfreeblocks = freeblock;
}

void World::remove_freeblock(FreeBlock* freeblock) {
  FreeBlock** free = &allfreeblocks;
  while (*free != nullptr) {
    if (*free == freeblock) {
      *free = (*free)->allfreeblocks;
      return;
    }
    free = &(*free)->allfreeblocks;
  }
}

#endif
