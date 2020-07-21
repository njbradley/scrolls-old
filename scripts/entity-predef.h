#ifndef ENTITY_PREDEF
#define ENTITY_PREDEF

#include <iostream>
#include "classes.h"
#include "blocks-predef.h"

//#include "items.h"
//#include "rendervec.h"
#include "items-predef.h"
#include "rendervec-predef.h"
#include "collider-predef.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
using namespace glm;

using namespace std;

class Entity { public:
    static constexpr float axis_gap = 0.2f;
    vec3 position;
		bool autojump = false;
    vec3 vel;
    vec2 angle;
    vec3 box1;
    vec3 box2;
    int health;
    float lastTime;
    float deltaTime;
    bool flying;
    vec3 dirs[6] = {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
    bool old_consts[7];
    bool consts[7]; // constraints for entity, posx, pos y, pos z, neg x, neg y, neg z
    bool alive;
    bool immune;
    World* world;
    
    Entity(World* nworld, vec3 pos, vec3 hitbox1, vec3 hitbox2);
    void timestep();
    void drop_ticks();
    virtual void calc_constraints();
    void find_colliders(vector<Collider*>* colliders);
    void get_nearby_entities(vector<DisplayEntity*>* colliders);
    void move(vec3 change, float deltaTime);
    void fall_damage(float velocity);
    void drag(bool do_drag, float deltaTime);
    virtual void on_timestep();
    virtual void tick();
    bool colliding(const Entity* other);
    virtual void kill();
};


class Player: public Entity {
	
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
	vec3 pointing;
	int selitem;
	
	public:
		ItemContainer inven;
		ItemContainer backpack;
		
		Player(World* newworld, vec3 pos);
    Player(World* newworld, istream& ifile);
    void save_to_file(ostream& ofile);
    virtual void die();
    virtual void calc_constraints();
		mat4 getViewMatrix();
		mat4 getProjectionMatrix();
		void right_mouse();
		void left_mouse();
    void raycast(Pixel** hit, vec3* hitpos, DisplayEntity** entity);
		void mouse_button();
		void computeMatricesFromInputs();
		void render_ui(MemVecs * uivecs);
};


class DisplayEntity: public Entity {
public:
  BlockContainer block;
  vec3 blockpos;
  vector<DisplayEntity*> limbs;
  MemVecs vecs;
  pair<int,int> render_index;
  bool render_flag;
  bool dead_falling;
  DisplayEntity(World* nworld, vec3 starting_pos, vec3 hitbox1, vec3 hitbox2, Block* newblock, vec3 blockpos,
    vector<DisplayEntity*> newlimbs);
  DisplayEntity(World* nworld, vec3 starting_pos, vec3 hitbox1, vec3 hitbox2, Block* newblock, vec3 blockpos);
  DisplayEntity(World* nworld, istream& ifile);
  ~DisplayEntity();
  void render(RenderVecs* allvecs);
  virtual void on_timestep();
  virtual void to_file(ostream& ofile);
  //void die();
};

class NamedEntity: public DisplayEntity {
public:
  string nametype;
  int pointing;
  NamedEntity(World* nworld, vec3 starting_pos, vec3 hitbox1, vec3 hitbox2, string name, vec3 newblockpos,
  vector<DisplayEntity*> limbs);
  NamedEntity(World* nworld, vec3 starting_pos, vec3 hitbox1, vec3 hitbox2, string name, vec3 newblockpos);
  NamedEntity(World* nworld, istream& ifile);
  Block* loadblock(string name);
  void on_timestep();
};

class FallingBlockEntity: public DisplayEntity, public Collider {
public:
  BlockGroup* group;
  FallingBlockEntity(World* nworld, BlockGroup* newgroup);
  FallingBlockEntity(World* nworld, istream& ifile);
  void calc_constraints();
  Block * get_global(int,int,int,int);
  vec3 get_position() const;
  void on_timestep();
  virtual void to_file(ostream& ofile);
  //virtual void to_file(ostream& ofile);
};

class EntityStorage { public:
  std::map<string,string> blocks;
  EntityStorage();
};

EntityStorage* entitystorage;

#endif
