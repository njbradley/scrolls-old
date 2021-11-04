#ifndef BASE_PLAYER_H
#define BASE_PLAYER_H

#include "classes.h"
#include "entity.h"

class Controls { public:
	PLUGIN_HEAD(Controls, ());
	
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
    void raycast(Block** hit, ivec3* dir, vec3* hitpos);
		void mouse_button();
		void computeMatricesFromInputs();
    void update_held_light();
		void render_ui(UIVecs* uivecs);
};

#endif
