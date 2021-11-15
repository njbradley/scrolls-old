#ifndef BASE_PLAYER_H
#define BASE_PLAYER_H

#include "classes.h"
#include "entity.h"
#include "blockdata.h"
#include "materials.h"

class Controls { public:
	BASE_PLUGIN_HEAD(Controls, ());
	
	int KEY_SHIFT;
	int KEY_CTRL;
	int KEY_ALT;
	int KEY_TAB;
	
	virtual bool key_pressed(int keycode) = 0;
	virtual bool mouse_button(int button) = 0;
	virtual ivec2 mouse_pos() = 0;
	virtual void mouse_pos(ivec2 newpos) = 0;
	virtual int scroll_rel() = 0;
};

extern Plugin<Controls> controls;

class Player: public Entity {
	
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
	vec3 pointing;
	int selitem;
	double break_progress = 0;
  ivec3 block_breaking;
  // GroupConnection* connection_breaking = nullptr;
  double attack_recharge = 0;
  double total_attack_recharge = 0.2;
  
  Block* placing_block = nullptr;
  FreeBlock* placing_freeblock = nullptr;
	FreeBlock* moving_freeblock = nullptr;
	float moving_range;
	vec3 moving_point;
  ivec3 placing_dir;
  vec2 placing_pointing;
	double timeout;
  
	public:
		ItemContainer inven;
		ItemContainer backpack;
		
		vec2 angle;
    double health = 0;
    double healing_health = 0;
    double damage_health = 0;
    double healing_speed = 0;
		
		bool spectator = false;
		Controls* controller = nullptr;
		
		Player();
		
		virtual void init();
		virtual void from_file(istream& ifile);
		virtual void to_file(ostream& ifile) const;
		mat4 getViewMatrix();
		mat4 getProjectionMatrix();
		void right_mouse(double dt);
		void left_mouse(double dt);
    void raycast(Block** hit, ivec3* dir, vec3* hitpos);
		void mouse_button();
		void die();
		virtual void tick(float curtime, float deltatime);
		virtual void timestep(float curtime, float deltatime);
		void computeMatricesFromInputs(float deltatime);
		void render_ui(UIVecs* uivecs);
		
		static BlockData blockdata;
		static Material material;
};

#endif
