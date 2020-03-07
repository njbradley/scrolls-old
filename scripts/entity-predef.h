#ifndef ENTITY_PREDEF
#define ENTITY_PREDEF

#include <iostream>
#include "classes.h"

//#include "items.h"
//#include "rendervec.h"
#include "items-predef.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
using namespace glm;

using namespace std;


class Entity { public:
    vec3 position;
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
    void calc_constraints(World* world);
    void move(vec3 change, float deltaTime);
    void fall_damage(float velocity);
    void drag(bool do_drag, float deltaTime);
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
		bool autojump = false;
		
		Player(vec3 pos, World* newworld);
    Player(istream* ifile);
    void save_to_file(ostream* ofile);
		mat4 getViewMatrix();
		mat4 getProjectionMatrix();
		void right_mouse();
		void left_mouse();
		void mouse_button();
		void computeMatricesFromInputs(World* nworld);
		void render_ui(RenderVecs * uivecs);
};



#endif
