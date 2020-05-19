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
  virtual Block * get_global(int,int,int,int) = 0;
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
    
    Entity(vec3 pos, vec3 hitbox1, vec3 hitbox2);
    void timestep(World* world);
    virtual void calc_constraints(World* world);
    void find_colliders(vector<Collider*>* colliders);
    void get_nearby_entities(vector<DisplayEntity*>* colliders);
    void move(vec3 change, float deltaTime);
    void fall_damage(float velocity);
    void drag(bool do_drag, float deltaTime);
    virtual void on_timestep(World* world);
};


class Player: public Entity {
	
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
	vec3 pointing;
	World* world;
	int selitem;
	
	public:
		ItemContainer inven;
		ItemContainer backpack;
		
		Player(vec3 pos, World* newworld);
    Player(istream& ifile);
    void save_to_file(ostream& ofile);
    void die();
		mat4 getViewMatrix();
		mat4 getProjectionMatrix();
		void right_mouse();
		void left_mouse();
    vec3 raycast(Pixel* hit, DisplayEntity* entity);
    void drop_ticks();
		void mouse_button();
		void computeMatricesFromInputs(World* nworld);
		void render_ui(MemVecs * uivecs);
};


class DisplayEntity: public Entity, public Collider {
public:
  Block* block;
  MemVecs vecs;
  pair<int,int> render_index;
  DisplayEntity(vec3 starting_pos, Block* newblock);
  Block * get_global(int,int,int,int);
  void render();
  void calc_constraints(World* world);
  void on_timestep(World* world);
};

class NamedEntity: public DisplayEntity {
public:
  string nametype;
  NamedEntity(vec3 starting_pos, string name);
  Block* loadblock(string name);
};

class EntityGroup {
  vector<Entity*> entities;
  void render(GLVecs* vecs);
  void add(Entity*);
};

class EntityStorage { public:
  std::map<string,string> blocks;
  EntityStorage();
};

EntityStorage* entitystorage;

#endif
