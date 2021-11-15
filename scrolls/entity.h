#ifndef ENTITY_PREDEF
#define ENTITY_PREDEF

#include "classes.h"
#include "plugins.h"
#include "blocks.h"
#include "items.h"


class Entity : public FreeBlock { public:
  BASE_PLUGIN_HEAD(Entity, ());
  
  Entity();
  virtual ~Entity();
  
  virtual void init();
  virtual void from_file(istream& ifile);
  virtual void to_file(ostream& ofile) const;
  
  virtual void set_position(vec3 pos, quat rot = quat(1,0,0,0));
  
  virtual void timestep(float curtime, float deltatime);
  virtual Entity* entity_cast();
  
  static Entity* create_from_id(istream& ifile);
};
/*
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
*/


#endif
