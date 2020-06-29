#ifndef ENTITY_PREDEF
#define ENTITY_PREDEF

#include <iostream>
#include "classes.h"

//#include "items.h"
//#include "rendervec.h"
#include "items-predef.h"
#include "rendervec-predef.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
using namespace glm;

using namespace std;

struct Collider {
  virtual Block * get_global(int x,int y,int z,int scale) = 0;
  // this is a function shared by all classes that hold
  // chunks/pixels in them
  // it returns a block that is at the global position xyz
  // and at scale scale
  // to make sure a pixel is returned, use scale 1 and call
  // get_pix() on the result
  virtual vec3 get_position() const = 0;
};

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
		mat4 getViewMatrix();
		mat4 getProjectionMatrix();
		void right_mouse();
		void left_mouse();
    void raycast(Pixel** hit, vec3* hitpos, DisplayEntity** entity);
    void drop_ticks();
		void mouse_button();
		void computeMatricesFromInputs();
		void render_ui(MemVecs * uivecs);
};


class DisplayEntity: public Entity, public Collider {
public:
  Block* block;
  map<vec3,Block*,vec3_comparator> limbs;
  MemVecs vecs;
  pair<int,int> render_index;
  bool render_flag;
  DisplayEntity(World* nworld, vec3 starting_pos, Block* newblock,
    map<vec3,Block*,vec3_comparator> newlimbs = map<vec3,Block*,vec3_comparator>());
  ~DisplayEntity();
  Block * get_global(int,int,int,int);
  vec3 get_position() const;
  void render();
  void calc_constraints();
  virtual void on_timestep();
  //void die();
};

class NamedEntity: public DisplayEntity {
public:
  string nametype;
  int pointing;
  NamedEntity(World* nworld, vec3 starting_pos, string name,
    map<vec3,string,vec3_comparator> limbnames);
  Block* loadblock(string name);
  map<vec3,Block*,vec3_comparator> loadlimbs(map<vec3,string,vec3_comparator>& names);
  void on_timestep();
};

class FallingBlockEntity: public DisplayEntity {
public:
  BlockGroup* group;
  FallingBlockEntity(World* nworld, BlockGroup* newgroup);
  void on_timestep();
};

class EntityStorage { public:
  std::map<string,string> blocks;
  EntityStorage();
};

EntityStorage* entitystorage;

#endif
