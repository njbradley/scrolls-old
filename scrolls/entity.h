#ifndef ENTITY_PREDEF
#define ENTITY_PREDEF

#include "classes.h"

#include "collider.h"
#include "items.h"

// Class that represents a rectangular prism, with a position, minimum, and maximum point.
// Immutable class
class Hitbox { public:
  vec3 position;
  vec3 negbox;
  vec3 posbox;
  quat rotation;
  vec3 velocity;
  vec3 angular_vel;
  
  Hitbox(vec3 pos, vec3 nbox, vec3 pbox, quat rot = quat(1,0,0,0), vec3 vel = vec3(0,0,0), vec3 angvel = vec3(0,0,0));
  Hitbox();
  
  void points(vec3* points) const;
  vec3 point1() const;
  vec3 point2() const;
  vec3 size() const;
  vec3 local_center() const;
  vec3 global_center() const;
  Hitbox boundingbox() const;
  vec3 dirx() const;
  vec3 diry() const;
  vec3 dirz() const;
  
  vec3 in_bounds(vec3 pos) const;
  
  vec3 transform_in(vec3 pos) const;
  quat transform_in(quat rot) const;
  vec3 transform_in_dir(vec3 pos) const;
  vec3 transform_out(vec3 pos) const;
  quat transform_out(quat rot) const;
  vec3 transform_out_dir(vec3 pos) const;
  
  Hitbox transform_in(Hitbox box) const;
  Hitbox transform_out(Hitbox box) const;
  
  // boxes collide when they overlap, so bordering cubes are not colliding
  float axis_projection(vec3 axis) const;
  bool collide(Hitbox other, float deltatime = 0, float* coltime = nullptr) const;
  
  bool contains_noedge(vec3 point) const;
  bool contains(vec3 point) const;
  // to be contained, all the points in the other box cant be outside the box
  // but they can be on the border.
  bool contains(Hitbox other) const;
  
  Hitbox timestep(float deltatime) const;
  Hitbox force(vec3 nforce) const;
  
  void debug_render(RenderVecs* transvecs);
  
  friend ostream& operator<<(ostream& ofile, const Hitbox& hitbox);
  
  static Hitbox boundingbox_points(vec3 pos, vec3* points, int num);
};

// Describes an impact between two hitboxes
// When no collision occurs, num_hitpoints is 0
// (casting to bool returns whether a colision occurs)
class ImpactManifold { public:
  Hitbox box1;
  Hitbox box2;
  vec3 hitpoints[4];
  int num_hitpoints;
  float time;
  
  // Creates fake manifolds with no collision
  // first one has no box or time at all,
  // second has one box and max time, and only
  // calls to newbox1 work.
  ImpactManifold();
  ImpactManifold(Hitbox nbox, float maxtime);
  // Creates a manfold and evaluates if the two boxes collides
  // if they do, num_hitpoints will be nonzero and time
  // will be the collision time
  ImpactManifold(Hitbox nbox1, Hitbox nbox2, float maxtime);
  
  // whether the boxes collide
  operator bool();
  
  // returns the new boxes after time
  // if dt >= time, then the boxes will be modified
  // with the results of the collision
  Hitbox newbox1(float dt);
  Hitbox newbox2(float dt);
  // these are shortcuts for newbox1(time)
  Hitbox newbox1();
  Hitbox newbox2();
};

class ImpactPlan { public:
  vector<ImpactManifold> impacts;
  Hitbox startbox;
  float time;
  
  ImpactPlan();
  ImpactPlan(FreeBlock* freeblock, float maxtime);
  
  Hitbox newbox(float dt);
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



#endif
