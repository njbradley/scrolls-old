#ifndef BLOCKS
#define BLOCKS

#include "blocks.h"

#include <cmath>
#include "rendervecs.h"
#include "blockdata.h"
#include "world.h"
#include "entity.h"
#include "blockiter.h"
#include "debug.h"
#include "graphics.h"
#include "player.h"
#include "fileformat.h"

const uint8 lightmax = 20;

/*
 *this is the file with classes for storing blocks. the main idea is blocks are stored in power of two containers. Block is the base virtual class,
 *and Pixel and Chunk derive from Block.
 *
 */


#define READ_LOCK ;
#define WRITE_LOCK ;

#ifdef SCROLLS_DEBUG
#define SCROLLS_BLOCK_CHECKS
#endif

#define ISPIXEL (!continues and pixel != nullptr)
#define ISCHUNK continues
#define ISUNDEF (!continues and pixel == nullptr)

#ifdef SCROLLS_BLOCK_CHECKS
#define ASSERT(cond) if (!(cond)) {ERR("ERR: assert failed: " #cond)}
#else
#define ASSERT(cond) ;
#endif


// Shorthand for:
// for (int i = 0; i < csize3; i ++) {
//    if (children[i] != nullptr) {
//      .....
#define CHILDREN_LOOP(varname) int varname = 0; varname < csize3; varname ++) if (children[varname] != nullptr

#define FREECHILDREN_LOOP(varname) FreeBlock* varname = freechild; varname != nullptr; varname = varname->next

Block::Block(): continues(false), pixel(nullptr) {
  // for (int i = 0; i < csize3; i ++) {
  //   children[i] = nullptr;
  // }
}

Block::Block(Block* childs[8]): continues(true) {
  for (int i = 0; i < csize3; i ++) {
    children[i] = childs[i];
  }
}

Block::Block(Pixel* pix): continues(false), pixel(pix) {
  pixel->set_block(this);
}

Block::~Block() {
  if (continues) {
    for (CHILDREN_LOOP(i)) {
      delete children[i];
    }
  } else if (pixel != nullptr) {
    // if (game->debugblock == pixel) {
    //   game->debugblock = nullptr;
    // }
    delete pixel;
  }
  for (FREECHILDREN_LOOP(free)) {
    delete free;
  }
}

bool Block::ischunk() const {
  return continues;
}

bool Block::ispixel() const {
  return !continues and pixel != nullptr;
}

bool Block::isundef() const {
  return !continues and pixel == nullptr;
}

void Block::incref() {
  refcount ++;
}

void Block::decref() {
  refcount --;
}

void Block::lock() {
  locked++;
}

void Block::unlock() {
  locked--;
  if (!locked) {
    if (flags & UPDATE_FLAG) {
      update();
      reset_flag(UPDATE_FLAG);
    }
    set_flag(flags);
  }
}

    
    

void Block::update() {
  if (locked) {
    set_flag(UPDATE_FLAG);
  } else {
    if (!continues and pixel != nullptr) {
      if (pixel->physicsbox != nullptr) {
        if (pixel->value == 0) {
          delete pixel->physicsbox;
          pixel->physicsbox = nullptr;
        } else {
          pixel->physicsbox->update();
        }
      }
    }
  }
}

void Block::on_change() {
  set_flag(RENDER_FLAG | LIGHT_FLAG | CHANGE_FLAG | CHANGE_PROP_FLAG);
}

void Block::set_parent(Block* block) {
  parent = block;
  on_change();
}

void Block::set_child(ivec3 pos, Block* block) {
  set_child(indexof(pos), block);
}

void Block::set_child(int index, Block* block) { ASSERT(ischunk())
  std::lock_guard<Block> guard(*this);
  if (children[index] != nullptr) {
    delete children[index];
  }
  children[index] = block;
  if (block != nullptr) {
    block->set_parent(this);
  }
}

Block* Block::swap_child(ivec3 pos, Block* block) {
  return swap_child(indexof(pos), block);
}

Block* Block::swap_child(int index, Block* block) { ASSERT(ischunk())
  std::lock_guard<Block> guard(*this);
  Block* old = children[index];
  children[index] = block;
  if (block != nullptr) {
    block->set_parent(this);
  }
  return old;
}

void Block::set_pixel(Pixel* pix) { ASSERT(ispixel() or isundef())
  std::lock_guard<Block> guard(*this);
  if (pix != nullptr) {
    pix->set_block(this);
  }
  if (pixel != nullptr) {
    delete pixel;
  }
  pixel = pix;
}

Pixel* Block::swap_pixel(Pixel* pix) { ASSERT(ispixel() or isundef())
  std::lock_guard<Block> guard(*this);
  if (pix != nullptr) {
    pix->set_block(this);
  }
  Pixel* old = pixel;
  pixel = pix;
  return old;
}


void Block::add_freechild(BlockView& view, FreeBlock* newfree) {
  // std::lock_guard<Block> guard(*this);
  
  FreeBlock** freedest = &freechild;
  while (*freedest != nullptr) {
    freedest = &(*freedest)->next;
  }
  
  *freedest = newfree;
  newfree->next = nullptr;
  newfree->set_parent(view);
  newfree->parent = nullptr;
  
  Block* curblock = this;
  while (curblock != nullptr) {
    curblock->freecount ++;
    curblock = curblock->parent;
  }
}

void Block::remove_freechild(FreeBlock* newfree) {
  // std::lock_guard<Block> guard(*this);
  FreeBlock** freedest = &freechild;
  
  while (*freedest != nullptr and *freedest != newfree) {
    freedest = &(*freedest)->next;
  }
  if (*freedest != nullptr) {
    FreeBlock* next = (*freedest)->next;
    *freedest = next;
    newfree->next = nullptr;
  } else {
    cout << "REMOVE NOT DOUNF" << endl;
    std::terminate();
  }
  
  Block* curblock = this;
  while (curblock != nullptr) {
    curblock->freecount --;
    curblock = curblock->parent;
  }
}




Block* Block::get(ivec3 pos) {
  return children[pos.x*csize*csize + pos.y*csize + pos.z];
}

Block* Block::get(int x, int y, int z) {
  return children[x*csize*csize + y*csize + z];
}

Block* Block::get(int index) {
  return children[index];
}

const Block* Block::get(ivec3 pos) const {
  return children[pos.x*csize*csize + pos.y*csize + pos.z];
}

const Block* Block::get(int x, int y, int z) const {
  return children[x*csize*csize + y*csize + z];
}

const Block* Block::get(int index) const {
  return children[index];
}

ivec3 Block::posof(int index) const {
  return ivec3(index/(csize*csize), index/csize%csize, index%csize);
}

int Block::indexof(ivec3 pos) {
  return pos.x*csize*csize + pos.y*csize + pos.z;
}

int Block::indexof(int x, int y, int z) {
  return x*csize*csize + y*csize + z;
}

void Block::divide() { ASSERT(!continues)
  std::lock_guard<Block> guard(*this);
  set_pixel(nullptr);
  continues = true;
  for (int i = 0; i < csize3; i ++) {
    children[i] = nullptr;
  }
  on_change();
}

void Block::subdivide() { ASSERT(!continues)
  std::lock_guard<Block> guard(*this);
  Pixel* pix = swap_pixel(nullptr);
  divide();
  for (int i = 0; i < csize3; i ++) {
    set_child(posof(i), new Block(new Pixel(pix->value, pix->direction)));
  }
  delete pix;
  on_change();
}

void Block::join() { ASSERT(ischunk())
  std::lock_guard<Block> guard(*this);
  for (CHILDREN_LOOP(i)) {
    delete children[i];
  }
  continues = false;
  pixel = nullptr;
  on_change();
}

void Block::to_file(ostream& ofile) const {
  for (FREECHILDREN_LOOP(free)) {
    ofile << blockformat::free;
    free->to_file(ofile);
  }
  if (continues) {
    ofile << blockformat::chunk;
    for (int i = 0; i < csize3; i ++) {
      if (children[i] != nullptr) {
        children[i]->to_file(ofile);
      } else {
        ofile << blockformat::null;
      }
    }
  } else {
    pixel->to_file(ofile);
  }
}

void Block::from_file(istream& ifile) {
  std::lock_guard<Block> guard(*this);
  while (ifile.peek() == blockformat::free) {
    ifile.get();
    FreeBlock* free;
    if (ifile.peek() == blockformat::entity) {
      free = Entity::create_from_id(ifile);
      // add_freechild(free);
    } else {
      free = new FreeBlock();
      // add_freechild(free);
      free->from_file(ifile);
    }
  }
  if (ifile.peek() == blockformat::chunk) {
    ifile.get();
    divide();
    for (int i = 0; i < csize3; i ++) {
      if (ifile.peek() == (blockformat::null)) {
        ifile.get();
        children[i] = nullptr;
      } else {
        Block* block = new Block();
        set_child(posof(i), block);
        block->from_file(ifile);
        // set_child(posof(i), new Block(ifile));
      }
    }
  } else {
    set_pixel(new Pixel(this, ifile));
  }
}

void Block::set_flag(uint8 flag) {
  Block* curblock = this;
  
  do {
    curblock->flags |= flag;
    flag &= PROPAGATING_FLAGS;
    if (curblock->locked) break;
    
    // if (curblock->parent == nullptr and curblock->freecontainer != nullptr) {
    //   curblock = curblock->freecontainer->highparent;
    // } else {
      curblock = curblock->parent;
    // }
  } while (curblock != nullptr);
}

void Block::reset_flag(uint8 flag) {
  flags &= ~flag;
}

void Block::set_all_flags(uint8 flag) {
  flags |= flag;
  if (continues) {
    for (CHILDREN_LOOP(i)) {
      children[i]->set_all_flags(flags);
    }
  }
  for (FREECHILDREN_LOOP(free)) {
    free->set_all_flags(flags);
  }
}

void Block::reset_all_flags(uint8 flag) {
  flags &= ~flag;
  if (continues) {
    for (CHILDREN_LOOP(i)) {
      children[i]->reset_all_flags(flags);
    }
  }
  for (FREECHILDREN_LOOP(free)) {
    free->reset_all_flags(flags);
  }
}






FreeBlock::FreeBlock() {
  
}

FreeBlock::FreeBlock(Movingbox newbox): Block(), box(newbox), lastbox(newbox), renderbox(newbox) {
  
}

FreeBlock::~FreeBlock() {
  if (physicsbody != nullptr) {
    delete physicsbody;
  }
  if (highparent.isvalid() and highparent.world != nullptr) {
    // highparent->world->remove_freeblock(this);
  }
}

void FreeBlock::to_file(ostream& ofile) const {
  Block::to_file(ofile);
  
  FileFormat::write_fixed(ofile, box.position.x);
  FileFormat::write_fixed(ofile, box.position.y);
  FileFormat::write_fixed(ofile, box.position.z);
  
  FileFormat::write_fixed(ofile, box.negbox.x);
  FileFormat::write_fixed(ofile, box.negbox.y);
  FileFormat::write_fixed(ofile, box.negbox.z);
  
  FileFormat::write_fixed(ofile, box.velocity.x);
  FileFormat::write_fixed(ofile, box.velocity.y);
  FileFormat::write_fixed(ofile, box.velocity.z);
  
  FileFormat::write_fixed(ofile, box.angularvel.x);
  FileFormat::write_fixed(ofile, box.angularvel.y);
  FileFormat::write_fixed(ofile, box.angularvel.z);
  
  FileFormat::write_fixed(ofile, box.rotation.w);
  FileFormat::write_fixed(ofile, box.rotation.x);
  FileFormat::write_fixed(ofile, box.rotation.y);
  FileFormat::write_fixed(ofile, box.rotation.z);
  
  ofile.put(fixed);
  ofile.put(asleep);
  ofile.put(allow_sleep);
  ofile.put(allow_rotation);
  ofile.put(allow_raycast);
}

void FreeBlock::from_file(istream& ifile) {
  Block::from_file(ifile);
  char type;
  
  FileFormat::read_fixed(ifile, &box.position.x);
  FileFormat::read_fixed(ifile, &box.position.y);
  FileFormat::read_fixed(ifile, &box.position.z);
  
  FileFormat::read_fixed(ifile, &box.negbox.x);
  FileFormat::read_fixed(ifile, &box.negbox.y);
  FileFormat::read_fixed(ifile, &box.negbox.z);
  
  FileFormat::read_fixed(ifile, &box.velocity.x);
  FileFormat::read_fixed(ifile, &box.velocity.y);
  FileFormat::read_fixed(ifile, &box.velocity.z);
  
  FileFormat::read_fixed(ifile, &box.angularvel.x);
  FileFormat::read_fixed(ifile, &box.angularvel.y);
  FileFormat::read_fixed(ifile, &box.angularvel.z);
  
  FileFormat::read_fixed(ifile, &box.rotation.w);
  FileFormat::read_fixed(ifile, &box.rotation.x);
  FileFormat::read_fixed(ifile, &box.rotation.y);
  FileFormat::read_fixed(ifile, &box.rotation.z);
  
  if (ifile.get()) {
    fix();
  }
  asleep = ifile.get();
  allow_sleep = ifile.get();
  allow_rotation = ifile.get();
  allow_raycast = ifile.get();
  
  box.posbox = box.negbox + 1.0f;
  renderbox = box;
  lastbox = box;
}

void FreeBlock::set_parent(const BlockView& view) {
  highparent = view;
  Block::set_parent(nullptr);
}

void FreeBlock::tick(float curtime, float deltatime) {
  if (physicsbody == nullptr) return;
  
  // float mag = 10 * scale;
  
  // if (controls->key_pressed('L')) {
  //   physicsbody->apply_impulse(vec3(-mag,0,0) * deltatime);
  // }
  // if (controls->key_pressed('J')) {
  //   physicsbody->apply_impulse(vec3(mag,0,0) * deltatime);
  // }
  // if (controls->key_pressed('I')) {
  //   physicsbody->apply_impulse(vec3(0,0,mag) * deltatime);
  // }
  // if (controls->key_pressed('V')) {
  //   physicsbody->apply_impulse(vec3(0,0,-mag) * deltatime);
  // }
  //
  // if (controls->key_pressed('U')) {
  //   physicsbody->apply_impulse(vec3(0,mag*2,0) * deltatime);
  // }
  
  // debuglines->render(box, vec3(1,1,1), "tick");
  // if (entity_cast() == nullptr or entity_cast()->get_plugin_id() != Player::plugindef()->id) {
    // debuglines->render(box);
  // }
}

void FreeBlock::timestep(float curtime, float deltatime) {
  
  // Movingbox newbox = box;
  // newbox.timestep(curtime - update_time);
  // renderbox = newbox;
  
  float ratio = std::max(0.0f, std::min((curtime - update_time) / World::tick_deltatime, 1.0f));
  renderbox = lastbox.lerp(box, ratio);
  // cout << renderbox << ' ' << curtime << ' ' << update_time << endl;
  // render_position();
  
  // box.timestep(deltatime);
  // set_box(box);
  
  // nextbox.dampen(0.001f,0.01f,0);
  
  // nextbox.velocity *= 1.0f / (1.0f + deltatime * 0.1f);
  // nextbox.angularvel *= 1.0f / (1.0f + deltatime * 0.1f);
}

Entity* FreeBlock::entity_cast() {
  return nullptr;
}

bool FreeBlock::set_box(Movingbox newbox, float curtime) {
  std::lock_guard<Block> guard(*this);
  
  if (highparent.isvalid() and !highparent.freebox().contains(newbox)) {
    // cout << "CHANGING higbox " << newbox << endl;
    
    vec3 guesspos = SAFEFLOOR3(newbox.global_midpoint());
    BlockView guess = highparent.get_global_exact(guesspos, highparent.scale);
    
    if (!guess.isvalid()) {
      return false;
    }
    
    // if (oldparent == guess) cout << "NO CHANGE IN HIGHPARENT " << endl;
    // cout << "CHANGING " << this << " parent: " << oldparent << " -> " << guess << endl;
    highparent->remove_freechild(this);
    guess->add_freechild(guess, this);
    highparent.try_join();
    highparent = guess;
  }
  
  if (curtime < 0) {
    // update_time = getTime();
    lastbox = newbox;
    renderbox = newbox;
    box = newbox;
  } else {
    lastbox = box;
    renderbox = box;
    box = newbox;
    update_time = curtime;
  }
  
  // render_position();
  // return;
  // for (Pixel* pix : iter()) {
  //   if (false and pix->lastvecs != nullptr) {
  //     // pix->render(pix->lastvecs, nullptr, 0xFF, false);
  //   } else {
  //     pix->parbl->set_flag(RENDER_FLAG);
  //   }
  // }
  
  // set_flag(RENDER_FLAG);
  set_all_flags(RENDER_FLAG);
  return true;
}



void FreeBlock::expand(ivec3 dir) { // Todo: this method only works for csize=2
  // std::lock_guard<Block> guard(*this);
  // // dir = (dir + 1) / 2;
  // Block* copyblock = new Block();
  // copyblock->swap(this);
  // divide();
  //
  // // expanding box and correcting highparent;
  // box.negbox -= vec3(scale * dir);
  // box.posbox += vec3(scale * (1 - dir));
  //
  // // box.position -= vec3(scale * dir);
  // // box.posbox += float(scale);
  //
  // if (!highparent.isvalid()) {
  //   set_parent(nullptr, world, parentpos, scale * csize);
  // } else {
  //   highparent->remove_freechild(this);
  //   highparent.step_up();
  //   highparent->add_freechild(this);
  // }
  // // set_parent(highparent->parent, world, parentpos, scale * csize);
  // set_box(box);
  //
  // set_child(dir, copyblock);
  //
  // on_change();
}





void FreeBlock::try_sleep() {
  if (box.velocity == vec3(0,0,0)
  and box.angularvel == vec3(0,0,0)) {
    asleep = true;
  }
}

void FreeBlock::force_sleep() {
  box.velocity = vec3(0,0,0);
  box.angularvel = vec3(0,0,0);
  asleep = true;
}

void FreeBlock::wake() {
  asleep = false;
}



void FreeBlock::fix() {
  box.velocity = vec3(0,0,0);
  box.angularvel = vec3(0,0,0);
  if (!fixed and highparent.isvalid() and highparent.world != nullptr) {
    // highparent->world->remove_freeblock(this);
  }
  fixed = true;
}

void FreeBlock::unfix() {
  if (fixed and highparent.isvalid() and highparent.world != nullptr) {
    // highparent->world->add_freeblock(this);
  }
  fixed = false;
}


Pixel::Pixel(int val, int direct, int njoints[6]):
value(val) {
  if (value != 0) {
    if (value > blockstorage.size()) {
      cout << "ERR: unknown char code " << int(value) << " (corrupted file?)" << endl;
    }
    if (direct == -1) {
      direction = blockstorage[value]->default_direction;
    } else {
      direction = direct;
    }
    if (direction == 255) {
      ERR("bad dir")
    }
  }
  if (njoints != nullptr) {
    for (int i = 0; i < 6; i ++) {
      joints[i] = njoints[i];
    }
  }
  reset_lightlevel();
}

Pixel::Pixel(Block* newblock, istream& ifile):
parbl(newblock) {
  bool value_set = false;
  direction = 0xff;
  while (!value_set and !ifile.eof()) {
    unsigned long int data = FileFormat::read_variable(ifile);
    char type = data % 4;
    data /= 4;
    if (type == blockformat::valuetype) {
      value = data;
      value_set = true;
    } else if (type == blockformat::dirtype) {
      direction = data;
    } else if (type == blockformat::jointstype) {
      for (int i = 5; i >= 0; i --) {
        joints[i] = data % 32;
        if (joints[i] > 0) {
          joints[i] += 6;
        }
        data /= 32;
      }
    }
  }
  if (direction == 0xff) {
    if (value == 0) {
      direction = 0;
    } else {
      direction = blockstorage[value]->default_direction;
    }
  }
  reset_lightlevel();
}

Pixel::~Pixel() {
  erase_render();
}

void Pixel::set_block(Block* nblock) {
  parbl = nblock;
}

void Pixel::rotate(int axis, int dir) {
  WRITE_LOCK;
  const int positive[18] {
    0, 2, 4, 3, 5, 1,
    5, 1, 0, 2, 4, 3,
    1, 3, 2, 4, 0, 5
  };
  const int negative[18] {
    0, 5, 1, 3, 2, 4,
    2, 1, 0, 5, 4, 0,
    4, 0, 2, 1, 3, 5
  };
  if (value != 0 and blockstorage[value]->rotation_enabled) {
    if (dir == 1) {
      direction = positive[axis*6 +direction];
    } else if (dir == -1) {
      direction = negative[axis*6 +direction];
    }
  }
  parbl->set_flag(Block::RENDER_FLAG);
}

void Pixel::rotate_from_origin(int* mats, int* dirs, int rotation) {
  const int mappings[6][6] { // gives the new location of every face for every possible rotation
    {0, 0, 0, 0, 0, 0},
    {1, 3, 2, 4, 0, 5},
    {2, 1, 3, 5, 4, 0},
    {3, 1, 5, 0, 4, 2},
    {4, 0, 2, 1, 3, 5},
    {5, 1, 0, 2, 4, 3}
  };
  const int rot_map[6][6] { // gives the rotation needed for each face
    {0, 0, 0, 0, 0, 0},
    {0, 0, -1, 0, 0, 1},
    {0, 1, 0, 0, -1, 0},
    {0, 2, 0, 0, 2, 0},
    {0, 2, 1, 0, 2, -1},
    {0, -1, 0, 0, 1, 0}
  };
  if (rotation == 0) {
    return;
  }
  int oldmats[6];
  int olddirs[6];
  for (int i = 0; i < 6; i ++) {
    oldmats[i] = mats[i];
    olddirs[i] = dirs[i];
  }
  for (int i = 0; i < 6; i ++) {
    mats[mappings[rotation][i]] = oldmats[i];
    dirs[mappings[rotation][i]] = olddirs[i] + rot_map[rotation][i];
  }
}

void Pixel::rotate_to_origin(int* mats, int* dirs, int rotation) {
  const int mappings[6][6] {
    {0, 0, 0, 0, 0, 0},
    {1, 3, 2, 4, 0, 5},
    {2, 1, 3, 5, 4, 0},
    {3, 1, 5, 0, 4, 2},
    {4, 0, 2, 1, 3, 5},
    {5, 1, 0, 2, 4, 3}
  };
  const int rot_map[6][6] {
    {0, 0, 0, 0, 0, 0},
    {0, 0, -1, 0, 0, 1},
    {0, 1, 0, 0, -1, 0},
    {0, 2, 2, 0, -2, -2},
    {0, 2, 1, 0, 2, -1},
    {0, -1, 0, 0, 1, 0}
  };
  if (rotation == 0) {
    return;
  }
  int oldmats[6];
  int olddirs[6];
  for (int i = 0; i < 6; i ++) {
    oldmats[i] = mats[i];
    olddirs[i] = dirs[i];
  }
  for (int i = 0; i < 6; i ++) {
    mats[i] = oldmats[mappings[rotation][i]];
    dirs[i] = olddirs[mappings[rotation][i]];
    dirs[i] -= rot_map[rotation][i];
  }
}

void Pixel::render(BlockView& view, RenderVecs* allvecs, RenderVecs* transvecs) {
  
  if (value != 0) {
    ivec3 gpos = view.globalpos;
    BlockData* blockdata = blockstorage[value];
    RenderData renderdata;
    
    bool exposed = false;
    
    Hitbox mybox = view.local_hitbox();
    FreeBlock* freecont = view.freecontainer;
    // while (freecont != nullptr) {
    //   mybox = freecont->renderbox.transform_out(mybox);
    //   freecont = freecont->highparent->freecontainer;
    // }
    
    // quat rot = parbl->getrotation();
    renderdata.pos.loc.pos = mybox.global_midpoint();
    renderdata.pos.loc.rot = mybox.rotation;
    renderdata.pos.loc.scale = view.scale;
    
    int mat[6];
    for (int i = 0; i < 6; i ++) {
      mat[i] = blockdata->texture[i];
    }
    int dirs[6] {0,0,0,0,0,0};
    
    if (direction != blockdata->default_direction) {
      rotate_to_origin(mat, dirs, blockdata->default_direction);
      rotate_from_origin(mat, dirs, direction);
    }
     
    int minscale = 1;
    int i = 0;
    for (int i = 0; i < 6; i ++) {
      ivec3 dir = dir_array[i];
      renderdata.type.faces[i].tex = 0;
      
      int num_blocks = 0;
      int blocklight = 0;
      int sunlight = 0;
      
      for (BlockView& sideview : view.iter_touching_side(dir)) {
        if (sideview->pixel->value == 0) {
          blocklight += sideview->pixel->blocklight;
          sunlight += sideview->pixel->sunlight;
          num_blocks ++;
        }
      }
      
      if (num_blocks > 0) {
        renderdata.type.faces[i].tex = mat[i];
        renderdata.type.faces[i].rot = dirs[i];
        renderdata.type.faces[i].blocklight = blocklight / num_blocks;
        renderdata.type.faces[i].sunlight = sunlight / num_blocks;
        exposed = true;
      }
    }
    
    if (exposed) {
      if (blockdata->transparent) {
        if (lastvecs == transvecs and !render_index.isnull()) {
          lastvecs->edit(render_index, renderdata);
        } else {
          if (!render_index.isnull()) {
            lastvecs->del(render_index);
          }
          render_index = transvecs->add(renderdata);
          lastvecs = transvecs;
        }
      } else {
        if (lastvecs == allvecs and !render_index.isnull()) {
          allvecs->edit(render_index, renderdata);
        } else {
          if (!render_index.isnull()) {
            lastvecs->del(render_index);
          }
          render_index = allvecs->add(renderdata);
          lastvecs = allvecs;
        }
      }
    } else if (!render_index.isnull()) {
      lastvecs->del(render_index);
      render_index = RenderIndex::npos;
    }
    
  } else if (!render_index.isnull()) {
    lastvecs->del(render_index);
    render_index = RenderIndex::npos;
  }
}


void Pixel::render_position(const BlockView& view) {
  if (render_index.isnull()) {
    parbl->set_flag(Block::RENDER_FLAG);
  } else {
    RenderPosData posdata;
    
    Hitbox mybox = view.local_hitbox();
    FreeBlock* freecont = view.freecontainer;
    // while (freecont != nullptr) {
      mybox = freecont->renderbox.transform_out(mybox);
      // freecont = freecont->highparent->freecontainer;
    // }
    
    posdata.loc.pos = mybox.global_midpoint();
    posdata.loc.rot = mybox.rotation;
    posdata.loc.scale = view.scale;
    
    lastvecs->edit(render_index, posdata);
  }
}


void Pixel::set(BlockView& view, int val, int newdirection, int newjoints[6]) {
  WRITE_LOCK;
  if (val == 0) {
    newdirection = 0;
  } else {
    if (newdirection == -1 or !blockstorage[val]->rotation_enabled) {
      newdirection = blockstorage[val]->default_direction;
    }
  }
  
  bool become_air = value != 0 and val == 0;
  bool become_solid = value == 0 and val != 0;
  value = val;
  
  if (become_air) {
    for (int i = 0; i < 6; i ++) {
      joints[i] = 0;
    }
  }
  
  direction = newdirection;
  
  // BlockGroup* oldgroup = nullptr;
  
  if (group != nullptr) {
    // oldgroup = group;
    // group->del(this);
  }
  
  // parbl->world->block_update(parbl->globalpos);

  reset_lightlevel();
  render_update();
  
  // for (ivec3 dir : dir_array) {
  //   for (BlockView& view : view.iter_touching_side(dir)) {
  //     view->pixel->render_update();
  //   }
  // }
  //
  parbl->update();
  
  /*
  Block* block;
  Pixel* lastpix = nullptr;
  int i = 0;
  for (ivec3 unit_dir : dir_array) {
    int inverse_index = (i > 2) ? i-3 : i+3;
    ivec3 dir = unit_dir * parbl->scale;
    block = parbl->get_global(parbl->globalpos + dir, parbl->scale);
    if (block != nullptr) {
      for (Pixel* pix : block->iter_side(-unit_dir)) {
        pix->render_update();
        world->block_update(pix->parbl->globalpos);
        
        if (become_air) {
          pix->joints[inverse_index] = 0;
        } else {
          if (pix->value != 0 and newjoints != nullptr) {
            pix->joints[inverse_index] = newjoints[i];
            joints[i] = newjoints[i];
          } else {
            joints[i] = 0;
          }
        }
        
        if (pix->group != nullptr) {
          int num_touching = std::min(pix->parbl->scale, parbl->scale);
          if (become_air) {
            pix->group->consts[inverse_index] -= num_touching * num_touching;
          } else if (become_solid) {
            pix->group->consts[inverse_index] += num_touching * num_touching;
          }
        }
        
        if (oldgroup != nullptr and pix->group == oldgroup) {
          if (lastpix != nullptr) {
            oldgroup->del(lastpix);
          }
          lastpix = pix;
        }
      }
    }
    i++;
  }
  if (lastpix != nullptr) {
    oldgroup->del(lastpix);
  }*/
}
  
void Pixel::render_update() {
  parbl->set_flag(Block::RENDER_FLAG | Block::LIGHT_FLAG);
}


void Pixel::erase_render() {
  if (render_index.index != -1) {
    lastvecs->del(render_index);
    render_index = RenderIndex::npos;
  }
}

void Pixel::lighting_update(BlockView& view, bool spread) {
  return;/*
  if (!spread) {
    calculate_blocklight();
    calculate_sunlight();
    return;
  }
  // sunlight = lightmax;
  // return;
  unordered_set<ivec3,ivec3_hash> poses;
  unordered_set<ivec3,ivec3_hash> next_poses;
  calculate_blocklight(&poses);
  while (poses.size() > 0) {
    for (ivec3 pos : poses) {
      Block* b = parbl->get_global(pos, 1);
      if (b != nullptr) b->pixel->calculate_blocklight(&next_poses);
    }
    poses.swap(next_poses);
    next_poses.clear();
  }
  
  int total = 0;
  int max_size = 0;
  poses.clear();
  next_poses.clear();
  calculate_sunlight(&poses);
  while (poses.size() > 0) {
    for (ivec3 pos : poses) {
      Block* b = parbl->get_global(pos, 1);
      if (b != nullptr) {
        b->pixel->calculate_sunlight(&next_poses);
      }
    }
    poses.swap(next_poses);
    next_poses.clear();
    if (max_size < poses.size()) {
      max_size = poses.size();
    }
    next_poses.reserve(poses.size());
    total += poses.size();
  }*/
}

void Pixel::reset_lightlevel() {
  lightsource = 0xff;
  sunlight = lightmax;
  return;
  if (value != 0) {
    sunlight = 0;
    blocklight = blockstorage[value]->lightlevel;
  } else {
    sunlight = 0;
    blocklight = 0;
  }
  entitylight = 0;
}

/*
void Pixel::calculate_sunlight(unordered_set<ivec3,ivec3_hash>* next_poses) {
  if ((value != 0 and !blockstorage[value]->transparent)) {
    return;
  }
  const int decrement = 1;
  int gx, gy, gz;
  gx = parbl->globalpos.x;
  gy = parbl->globalpos.y;
  gz = parbl->globalpos.z;
  const ivec3 dirs[] = {{-1,0,0}, {0,-1,0}, {0,0,-1}, {1,0,0}, {0,1,0}, {0,0,1}};
  ivec3 tilepos = SAFEDIV(parbl->globalpos, World::chunksize);
  int oldsunlight = sunlight;
  sunlight = 0;
  for (ivec3 dir : dirs) {
    int newdec = dir.y == 1 ? 0 : decrement;
    int oppdec = dir.y == -1 ? 0 : decrement;
    Block* block = parbl->get_global(parbl->globalpos + dir*parbl->scale, parbl->scale);
    // Block* block = parbl->get_global(gx+dir.x*parbl->scale, gy+dir.y*parbl->scale, gz+dir.z*parbl->scale, parbl->scale);
    if (block != nullptr) {
      for (Pixel* pix : block->iter_side(-dir)) {
        // if (pix->tile != tile and dir.y == 1 and pix->tile->lightflag) {
          // sunlight = lightmax;
        // } else if (pix->tile != tile and dir.y == -1 and pix->sunlight == lightmax) {
          //sunlight = lightmax;
        // } else
        if (pix->sunlight - newdec*pix->parbl->scale > sunlight and pix->sunlight + oppdec*parbl->scale != oldsunlight) {
          sunlight = pix->sunlight - newdec*pix->parbl->scale;
        }
      }
    } else if (dir.y == 1) {
      sunlight = lightmax;
    }
  }
  
  bool changed = oldsunlight != sunlight;
  for (ivec3 dir : dirs) {
    int newdec = dir.y == -1 ? 0 : decrement;
    Block* block = parbl->get_global(parbl->globalpos + dir*parbl->scale, parbl->scale);
    // Block* block = parbl->get_global(gx+dir.x*parbl->scale, gy+dir.y*parbl->scale, gz+dir.z*parbl->scale, parbl->scale);
    // if (block != nullptr and !(*block->iter().begin())->tile->fully_loaded) {
    //   block = nullptr;
    // }
    if (block != nullptr) {
      for (Pixel* pix : block->iter_side(-dir)) {
        if (pix->sunlight < sunlight - newdec*parbl->scale or (pix->sunlight + newdec*parbl->scale == oldsunlight and sunlight < oldsunlight)) {
          if (tilepos == SAFEDIV(pix->parbl->globalpos, World::chunksize) and next_poses != nullptr) {
            next_poses->emplace(pix->parbl->globalpos);
          } else {
            pix->parbl->set_flag(Block::LIGHT_FLAG);
          }
        } else if (tilepos != SAFEDIV(pix->parbl->globalpos, World::chunksize) and dir.y == -1 and pix->sunlight == lightmax) {
          pix->parbl->set_flag(Block::LIGHT_FLAG);
        }
        if (changed) pix->parbl->set_flag(Block::RENDER_FLAG);
      }
    }
  }
}

void Pixel::calculate_blocklight(unordered_set<ivec3,ivec3_hash>* next_poses) {
  if ((value != 0 and !blockstorage[value]->transparent)) {
    return;
  }
  const int decrement = 1;
  int gx, gy, gz;
  gx = parbl->globalpos.x;
  gy = parbl->globalpos.y;
  gz = parbl->globalpos.z;
  ivec3 tilepos = SAFEDIV(parbl->globalpos, World::chunksize);
  const ivec3 dirs[] = {{-1,0,0}, {0,-1,0}, {0,0,-1}, {1,0,0}, {0,1,0}, {0,0,1}};
  int oldblocklight = blocklight;
  blocklight = entitylight;
  int oldsource = lightsource;
  lightsource = -1;
  int index = 0;
  for (ivec3 dir : dirs) {
    Block* block = parbl->get_global(parbl->globalpos + dir*parbl->scale, parbl->scale);
    // Block* block = parbl->get_global(gx+dir.x*parbl->scale, gy+dir.y*parbl->scale, gz+dir.z*parbl->scale, parbl->scale);
    if (block != nullptr) {
      int inverse_index = index<3 ? index + 3 : index - 3;
      for (Pixel* pix : block->iter_side(-dir)) {
        if (pix->blocklight - decrement*pix->parbl->scale > blocklight and pix->lightsource != inverse_index) {
          blocklight = pix->blocklight - decrement*pix->parbl->scale;
          lightsource = index;
        }
      }
    }
    index ++;
  }
  
  bool changed = oldblocklight != blocklight;
  index = 0;
  for (ivec3 dir : dirs) {
    Block* block = parbl->get_global(parbl->globalpos + dir*parbl->scale, parbl->scale);
    // Block* block = parbl->get_global(gx+dir.x*parbl->scale, gy+dir.y*parbl->scale, gz+dir.z*parbl->scale, parbl->scale);
    if (block != nullptr) {
      int inverse_index = index<3 ? index + 3 : index - 3;
      for (Pixel* pix : block->iter_side(-dir)) {
        if (pix->blocklight < blocklight - decrement*parbl->scale or (pix->lightsource == inverse_index and blocklight < oldblocklight)) {
          if (tilepos == SAFEDIV(pix->parbl->globalpos, World::chunksize) and next_poses != nullptr) {
            next_poses->emplace(pix->parbl->globalpos);
          } else {
            pix->parbl->set_flag(Block::LIGHT_FLAG);
          }
        }
        if (changed) pix->parbl->set_flag(Block::RENDER_FLAG);
      }
    }
    index ++;
  }
}*/

BlockData* Pixel::data() const {
  if (value == 0) {
    return nullptr;
  } else {
    return blockstorage[value];
  }
}

void Pixel::to_file(ostream& of) {
  int jointval = 0;
  for (int i = 0; i < 6; i ++) {
    jointval *= 32;
    if (joints[i] > 6) {
      jointval += joints[i] - 6;
    }
  }
  if (jointval != 0) {
    FileFormat::write_variable(of, (jointval<<2) | blockformat::jointstype);
  }
  if (value != 0 and direction != blockstorage[value]->default_direction) {
    FileFormat::write_variable(of, (direction<<2) | blockformat::dirtype);
  }
  FileFormat::write_variable(of, (value<<2) | blockformat::valuetype);
}













template <typename BlockT>
BlockViewTempl<BlockT>::BlockViewTempl(): world(nullptr), curblock(nullptr), parent(nullptr), scale(-1) {
  
}

template <typename BlockT>
BlockViewTempl<BlockT>::BlockViewTempl(ContainerT* nworld): world(nworld) {
  
}

// template <typename BlockT>
// BlockViewTempl<BlockT>::BlockViewTempl(BlockT* block): curblock(block), parent(block->parent), world(nullptr) {
//
// }

template <typename BlockT>
BlockViewTempl<BlockT>::BlockViewTempl(const BlockViewTempl<BlockT>& other):
world(other.world), parent(other.parent), curblock(other.curblock), freecontainer(other.freecontainer),
globalpos(other.globalpos), scale(other.scale) {
  if (isvalid()) {
    incref(curblock);
    BlockT* block = parent;
    FreeBlockT* freecont = freecontainer;
    while (block != nullptr) {
      incref(block);
      block = block->parent;
      if (block == nullptr and freecont != nullptr) {
        block = freecont->highparent.curblock;
        freecont = freecont->highparent.freecontainer;
      }
    }
  }
}

template <typename BlockT>
BlockViewTempl<BlockT>::BlockViewTempl(BlockViewTempl<BlockT>&& other) {
  swap(other);
}

template <typename BlockT>
BlockViewTempl<BlockT>::~BlockViewTempl() {
  if (isvalid()) {
    decref(curblock);
    while (parent != nullptr) {
      decref(parent);
      parent = parent->parent;
      if (parent == nullptr and freecontainer != nullptr) {
        parent = freecontainer->highparent.curblock;
        freecontainer = freecontainer->highparent.freecontainer;
      }
    }
  }
}

template <typename BlockT>
BlockViewTempl<BlockT>& BlockViewTempl<BlockT>::operator=(BlockViewTempl<BlockT> other) {
  swap(other);
  return *this;
}

template <typename BlockT>
BlockViewTempl<BlockT>& BlockViewTempl<BlockT>::operator=(BlockViewTempl<BlockT>&& other) {
  swap(other);
  return *this;
}

template <typename BlockT>
bool BlockViewTempl<BlockT>::operator==(BlockViewTempl<BlockT>& other) const {
  return curblock == other.curblock;
}

template <typename BlockT>
void BlockViewTempl<BlockT>::incref(BlockT* block) {
  
}

template <typename BlockT>
void BlockViewTempl<BlockT>::decref(BlockT* block) {
  
}


template <typename BlockT>
void BlockViewTempl<BlockT>::swap(BlockViewTempl<BlockT>& other) {
  using std::swap;
  
  swap(parent, other.parent);
  swap(curblock, other.curblock);
  swap(freecontainer, other.freecontainer);
  swap(world, other.world);
  swap(globalpos, other.globalpos);
  swap(scale, other.scale);
  swap(in_freecontainer, other.in_freecontainer);
}

template <typename BlockT>
ivec3 BlockViewTempl<BlockT>::parentpos() const { ASSERT(isvalid());
  return SAFEMOD(globalpos, scale*2) / scale;
}

template <typename BlockT>
bool BlockViewTempl<BlockT>::isvalid() const {
  return scale > 0;
}

template <typename BlockT>
bool BlockViewTempl<BlockT>::isrelative() const {
  return world != nullptr;
}

template <typename BlockT>
bool BlockViewTempl<BlockT>::isfreeblock() const {
  return in_freecontainer and freecontainer == curblock;
}

template <typename BlockT>
Hitbox BlockViewTempl<BlockT>::local_hitbox() const {
  ASSERT(isvalid() and !isrelative());
  return Hitbox(vec3(0,0,0), globalpos, globalpos + scale);
}

template <typename BlockT>
Hitbox BlockViewTempl<BlockT>::local_freebox() const {
  ASSERT(isvalid() and !isrelative());
  return Hitbox(vec3(0,0,0), vec3(globalpos) - scale/2.0f, vec3(globalpos) + scale*3/2.0f);
}

template <typename BlockT>
Hitbox BlockViewTempl<BlockT>::hitbox() const {
  ASSERT(isvalid() and !isrelative());
  if (freecontainer == nullptr) {
    return local_hitbox();
  } else {
    return freecontainer->box.transform_out(local_hitbox());
  }
}

template <typename BlockT>
Hitbox BlockViewTempl<BlockT>::freebox() const{
  ASSERT(isvalid() and !isrelative());
  if (freecontainer == nullptr) {
    return local_freebox();
  } else {
    return freecontainer->box.transform_out(local_freebox());
  }
}

template <typename BlockT>
Movingbox BlockViewTempl<BlockT>::movingbox() const {
  ASSERT(isvalid() and !isrelative());
  if (freecontainer == nullptr) {
    return Movingbox(hitbox(), vec3(0,0,0), vec3(0,0,0), std::numeric_limits<float>::infinity());
  } else {
    Movingbox newbox = freecontainer->box;
    newbox.negbox = vec3(globalpos) - scale/2.0f;
    newbox.posbox = vec3(globalpos) + scale/2.0f;
    return newbox;
  }
}

template <typename BlockT>
const Block* BlockViewTempl<BlockT>::operator->() const { ASSERT(isvalid());
  return curblock;
}

template <typename BlockT>
void BlockViewTempl<BlockT>::invalidate() {
  scale = -1;
}

template <typename BlockT>
bool BlockViewTempl<BlockT>::step_down(ivec3 pos) { ASSERT(isvalid());
  if (curblock != nullptr and curblock->continues) {
    parent = curblock;
    curblock = curblock->get(pos);
    incref(curblock);
    scale /= csize;
    globalpos += pos * scale;
    return true;
  }
  return false;
}

template <typename BlockT>
bool BlockViewTempl<BlockT>::step_down(int x, int y, int z) { ASSERT(isvalid());
  if (curblock != nullptr and curblock->continues) {
    parent = curblock;
    curblock = curblock->get(x,y,z);
    incref(curblock);
    scale /= csize;
    globalpos += ivec3(x,y,z) * scale;
    return true;
  }
  return false;
}

template <typename BlockT>
bool BlockViewTempl<BlockT>::step_down_free() { ASSERT(isvalid());
  if (curblock != nullptr and !free) {
    parent = curblock;
    freecontainer = curblock->freechild;
    curblock = freecontainer;
    incref(curblock);
    in_freecontainer = true;
    scale /= csize;
    globalpos = ivec3(0,0,0);
    return true;
  }
  return false;
}

template <typename BlockT>
bool BlockViewTempl<BlockT>::step_up() { ASSERT(isvalid());
  if (parent != nullptr) {
    decref(curblock);
    curblock = parent;
    parent = parent->parent;
    scale *= csize;
    globalpos = SAFEDIV(globalpos, scale) * scale;
    return true;
  }
  return false;
}

template <typename BlockT>
bool BlockViewTempl<BlockT>::step_up_free() { ASSERT(isvalid());
  if (isfreeblock()) {
    const BlockView* view = &freecontainer->highparent;
    curblock = view->curblock;
    parent = view->parent;
    scale = view->scale;
    globalpos = view->globalpos;
    freecontainer = view->freecontainer;
    in_freecontainer = view->in_freecontainer;
    return true;
  }
  return false;
}
    

template <typename BlockT>
bool BlockViewTempl<BlockT>::step_side(ivec3 pos) { ASSERT(isvalid());
  decref(curblock);
  curblock = parent->get(pos);
  incref(curblock);
  globalpos = SAFEDIV(globalpos, scale*2) * (scale*2) + pos * scale;
  return true;
}

template <typename BlockT>
bool BlockViewTempl<BlockT>::step_side(int x, int y, int z) { ASSERT(isvalid());
  decref(curblock);
  curblock = parent->get(x,y,z);
  incref(curblock);
  globalpos = SAFEDIV(globalpos, scale) * scale + ivec3(x,y,z) * scale;
  return true;
}

template <typename BlockT>
bool BlockViewTempl<BlockT>::step_side_free() { ASSERT(isvalid());
  if (isfreeblock() and curblock != nullptr) {
    decref(curblock);
    freecontainer = freecontainer->next;
    curblock = freecontainer;
    incref(curblock);
    return true;
  }
  return true;
}
    

template <typename BlockT>
BlockViewTempl<BlockT> BlockViewTempl<BlockT>::get_global(ivec3 pos, int w) {
  BlockViewTempl<BlockT> view (world);
  view.moveto(pos, w);
  return view;
}

template <typename BlockT>
bool BlockViewTempl<BlockT>::moveto(ivec3 pos, int w) {
  if (isvalid()) {
    while (SAFEDIV(pos, scale) != SAFEDIV(globalpos, scale) or scale < w) {
      if (!step_up()) {
        if (freecontainer != nullptr) {
          return false;
        } else if (world != nullptr) {
          ivec3 tilepos = SAFEDIV(pos, World::chunksize);
          curblock = world->get_child(tilepos);
          scale = World::chunksize;
          globalpos = tilepos * scale;
        } else {
          return false;
        }
      }
    }
  } else {
    ivec3 tilepos = SAFEDIV(pos, World::chunksize);
    curblock = world->get_child(tilepos);
    scale = World::chunksize;
    globalpos = tilepos * scale;
  }
  
  while (scale > w) {
    ivec3 rem = SAFEMOD(pos, scale) / (scale / csize);
    if (!step_down(rem)) break;
  }
  
  return true;
}



BlockView::BlockView(const BlockViewTempl<Block>& other): BlockViewTempl<Block>(other) {
  
}

Block* BlockView::operator->() {
  return curblock;
}

BlockView BlockView::get_global_exact(ivec3 pos, int w) {
  BlockView view(world);
  view.moveto_exact(pos, w);
  return view;
}

bool BlockView::moveto_exact(ivec3 pos, int w) {
  if (isvalid()) {
    while (SAFEDIV(pos, scale) != SAFEDIV(globalpos, scale) or scale < w) {
      if (!step_up()) {
        if (freecontainer != nullptr) {
          return false;
        } else if (world != nullptr) {
          ivec3 tilepos = SAFEDIV(pos, World::chunksize);
          curblock = world->get_child(tilepos);
          globalpos = tilepos * World::chunksize;
        } else {
          return false;
        }
      }
    }
  } else {
    ivec3 tilepos = SAFEDIV(pos, World::chunksize);
    curblock = world->get_child(tilepos);
    scale = World::chunksize;
    globalpos = tilepos * scale;
  }
  
  while (scale > w) {
    ivec3 rem = SAFEMOD(pos, scale) / (scale / csize);
    if (!step_down(rem)) {
      subdivide();
      step_down(rem);
    }
  }
  
  return true;
}

void BlockView::set(Block* block) {
  set_curblock(block);
}

void BlockView::set(Blocktype val, int direc) {
  if (curblock == nullptr) {
    set_curblock(new Block(new Pixel(val, direc)));
  } else if (curblock->continues) {
    curblock->join();
    curblock->set_pixel(new Pixel(val, direc));
  } else {
    curblock->pixel->set(*this, val, direc);
  }
}

void BlockView::set_curblock(Block* block) {
  if (curblock != nullptr) delete curblock;
  if (parent == nullptr) {
    world->set_child(parentpos(), block);
  } else {
    parent->set_child(parentpos(), block);
  }
}

Block* BlockView::swap_curblock(Block* block) {
  Block* oldblock = curblock;
  if (parent == nullptr) {
    ivec3 tilepos = SAFEDIV(globalpos, scale);
    if (world->get_child(tilepos) != nullptr) delete world->get_child(tilepos);
    world->set_child(tilepos, block);
  } else {
    parent->swap_child(parentpos(), block);
  }
  return oldblock;
}

void BlockView::subdivide() {
  if (curblock == nullptr) {
    curblock = new Block();
    curblock->divide();
  } else if (curblock->continues = false) {
    Pixel* pix = curblock->swap_pixel(nullptr);
    curblock->divide();
    for (int i = 0; i < csize3; i ++) {
      curblock->set_child(i, new Block(new Pixel(pix->value, pix->direction)));
    }
  }
}

void BlockView::try_join() {
  Pixel* pix = nullptr;
  for (BlockView& view : iter()) {
    if (view->freechild != nullptr) {
      return;
    }
    if (!view->continues) {
      if (pix == nullptr) {
        pix = view->pixel;
      } else if (pix->value != view->pixel->value) {
        return;
      }
    }
  }
  
  if (pix == nullptr) {
    set_curblock(nullptr);
  } else {
    curblock->join();
    curblock->set_pixel(new Pixel(pix->value));
  }
}


BlockIterable<PixelIterator<BlockView>> BlockView::iter() {
  return BlockIterable<PixelIterator<BlockView>> (*this);
}

BlockIterable<DirPixelIterator<BlockView>> BlockView::iter_side(ivec3 dir) {
  return BlockIterable<DirPixelIterator<BlockView>> (*this, dir);
}

BlockSideIterable<BlockView> BlockView::iter_touching_side(ivec3 dir) {
  return BlockSideIterable<BlockView>(*this, dir);
}


ConstBlockView::ConstBlockView(const BlockViewTempl<const Block>& other): BlockViewTempl<const Block>(other) {
  
}

ConstBlockView::ConstBlockView(const BlockView& other) {
  parent = other.parent;
  curblock = other.curblock;
  freecontainer = other.freecontainer;
  world = other.world;
  globalpos = other.globalpos;
  scale = other.scale;
  in_freecontainer = other.in_freecontainer;
}

BlockIterable<PixelIterator<ConstBlockView>> ConstBlockView::iter() {
  return BlockIterable<PixelIterator<ConstBlockView>> (*this);
}

BlockIterable<DirPixelIterator<ConstBlockView>> ConstBlockView::iter_side(ivec3 dir) {
  return BlockIterable<DirPixelIterator<ConstBlockView>> (*this, dir);
}

BlockSideIterable<ConstBlockView> ConstBlockView::iter_touching_side(ivec3 dir) {
  return BlockSideIterable<ConstBlockView>(*this, dir);
}

template class BlockViewTempl<Block>;
template class BlockViewTempl<const Block>;




#endif
