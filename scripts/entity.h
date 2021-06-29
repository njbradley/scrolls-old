#ifndef ENTITY_PREDEF
#define ENTITY_PREDEF

#include "classes.h"

#include "collider.h"
#include "blocks.h"
#include "items.h"
#include "rendervec.h"

// Class that represents a rectangular prism, with a position, minimum, and maximum point.
// the position is not that useful here, it is mostly needed for entities and freehitboxes
class Hitbox { public:
  vec3 position;
  vec3 negbox;
  vec3 posbox;
  
  Hitbox(vec3 pos, vec3 nbox, vec3 pbox);
  
  void points(vec3* points);
  vec3 local_center();
  vec3 global_center();
  
  vec3 transform_in(vec3 pos);
  quat transform_in(quat rot);
  vec3 transform_in_dir(vec3 pos);
  vec3 transform_out(vec3 pos);
  quat transform_out(quat rot);
  vec3 transform_out_dir(vec3 pos);
  
  Hitbox transform_in(Hitbox box);
  FreeHitbox transform_in(FreeHitbox box);
  Hitbox transform_out(Hitbox box);
  FreeHitbox transform_out(FreeHitbox box);
  
  // boxes collide when they overlap, so bordering cubes are not colliding
  bool collide(Hitbox other);
  bool collide(FreeHitbox other);
  
  bool contains_noedge(vec3 point);
  bool contains(vec3 point);
  // to be contained, all the points in the other box cant be outside the box
  // but they can be on the border.
  bool contains(Hitbox other);
  bool contains(FreeHitbox other);
  
  friend ostream& operator<<(ostream& ofile, const Hitbox& hitbox);
  
  static Hitbox boundingbox_points(vec3 pos, vec3* points, int num);
};

// hitbox that is rotated freely.
// the position is the rotation point
class FreeHitbox : public Hitbox { public:
  quat rotation;
  
  FreeHitbox(Hitbox box, quat rot);
  FreeHitbox(vec3 pos, vec3 nbox, vec3 pbox, quat rot);
  
  void points(vec3* points);
  Hitbox boundingbox();
  vec3 dirx();
  vec3 diry();
  vec3 dirz();
  vec3 global_center();
  
  vec3 transform_in(vec3 pos);
  quat transform_in(quat rot);
  vec3 transform_in_dir(vec3 pos);
  vec3 transform_out(vec3 pos);
  quat transform_out(quat rot);
  vec3 transform_out_dir(vec3 pos);
  
  FreeHitbox transform_in(Hitbox box);
  FreeHitbox transform_in(FreeHitbox box);
  FreeHitbox transform_out(Hitbox box);
  FreeHitbox transform_out(FreeHitbox box);
  
  bool collide(Hitbox other);
  bool collide(FreeHitbox other);
  
  bool contains_noedge(vec3 point);
  bool contains(vec3 point);
  
  bool contains(Hitbox other);
  bool contains(FreeHitbox other);
};

class Entity { public:
    static constexpr float axis_gap = 0.2f;
    vec3 position;
		bool autojump = false;
    vec3 vel;
    vec2 angle;
    vec3 box1;
    vec3 box2;
    float lastTime;
    float deltaTime;
    float density = 1.0f;
    bool flying;
    vec3 dirs[6] = {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
    bool old_consts[7] = {false, false, false, false, false, false};
    bool consts[7]; // constraints for entity, posx, pos y, pos z, neg x, neg y, neg z
    Material* mat_consts[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    bool alive;
    bool immune;
    bool spectator = false;
    World* world;
    double max_health;
    double health;
    double healing_health = 0;
    double damage_health = 0;
    double healing_speed = 0;
    bool in_water = false;
    double step_distance = 0;
    
    Entity(World* nworld, vec3 pos, vec3 hitbox1, vec3 hitbox2);
    void timestep();
    void drop_ticks();
    bool is_solid_block(Block* block);
    virtual void calc_constraints();
    void find_colliders(vector<Collider*>* colliders);
    void get_nearby_entities(vector<DisplayEntity*>* colliders);
    void move(vec3 change, float deltaTime);
    void fall_damage(float velocity, float multiplier);
    void drag(bool do_drag, float deltaTime);
    void damage(double amount);
    void use_stamina(double amount);
    void heal(double amount, double speed);
    virtual void on_timestep(double deltatime);
    virtual void tick();
    bool colliding(const Entity* other);
    virtual void kill();
};


class DisplayEntity: public Entity {
public:
  BlockContainer block;
  vec3 blockpos;
  vector<DisplayEntity*> limbs;
  MemVecs vecs;
  RenderIndex render_index;
  bool render_flag;
  bool dead_falling;
  int sunlight = 10;
  int blocklight = 0;
  int emitted_light = 0;
  bool emitted_light_flag = false;
  ivec3 lit_block;
  DisplayEntity(World* nworld, vec3 starting_pos, vec3 hitbox1, vec3 hitbox2, Block* newblock, vec3 blockpos,
    vector<DisplayEntity*> newlimbs);
  DisplayEntity(World* nworld, vec3 starting_pos, vec3 hitbox1, vec3 hitbox2, Block* newblock, vec3 blockpos);
  DisplayEntity(World* nworld, istream& ifile);
  ~DisplayEntity();
  virtual void render(RenderVecs* allvecs);
  virtual void calc_light(vec3 offset, vec2 ang);
  virtual void on_timestep(double deltatime);
  virtual void to_file(ostream& ofile);
  //void die();
};

class Player: public DisplayEntity {
	
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
	vec3 pointing;
	int selitem;
	double break_progress = 0;
  ivec3 block_breaking;
  GroupConnection* connection_breaking = nullptr;
  double attack_recharge = 0;
  double total_attack_recharge = 0.2;
  RenderIndex block_breaking_render_index;
  MemVecs block_breaking_vecs;
  int block_breaking_tex;
  
  Block* placing_block = nullptr;
  FreeBlock* placing_freeblock = nullptr;
  ivec3 placing_dir;
  vec2 placing_pointing;
  
	public:
		ItemContainer inven;
		ItemContainer backpack;
		
		Player(World* newworld, vec3 pos);
    Player(World* newworld, istream& ifile);
    void init_block_breaking_vecs();
    void set_block_breaking_vecs(Pixel* pix, ivec3 hitpos, int level);
    void save_to_file(ostream& ofile);
    virtual void die();
    //virtual void calc_constraints();
		mat4 getViewMatrix();
		mat4 getProjectionMatrix();
		void right_mouse(double dt);
		void left_mouse(double dt);
    void raycast(Pixel** hit, vec3* hitpos, DisplayEntity** entity);
    void raycast(Block** hit, ivec3* dir, vec3* hitpos);
		void mouse_button();
		void computeMatricesFromInputs();
    void update_held_light();
		void render_ui(MemVecs * uivecs);
};

#endif
