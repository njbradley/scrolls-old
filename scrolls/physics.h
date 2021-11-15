#ifndef SCROLLS_PHYSICS
#define SCROLLS_PHYSICS

#include "classes.h"
#include "plugins.h"

class Line { public:
  vec3 position;
  vec3 direction;
  
  Line();
  Line(vec3 pos, vec3 dir);
  
  bool contains(vec3 point) const;
  
  bool collides(Line other) const;
  vec3 collision(Line other) const;
  
  operator bool() const;
  
  static Line from_points(vec3 point1, vec3 point2);
};


class Plane { public:
  vec3 position;
  vec3 normal;
  
  Plane();
  Plane(vec3 pos, vec3 norm);
  
  bool contains(vec3 point) const;
  
  bool collides(Line line) const;
  vec3 collision(Line line) const;
  bool collides(Plane other) const;
  Line collision(Plane other) const;
  
  operator bool() const;
  
  static Plane from_points(vec3 point1, vec3 point2, vec3 point3);
};

  
// class that represents a point with mass and velocity
class Movingpoint { public:
  vec3 position;
  vec3 velocity;
  float mass;
  
  Movingpoint();
  Movingpoint(vec3 pos, float mass);
  explicit Movingpoint(vec3 pos, vec3 vel = vec3(0,0,0), float nmass = 1);
  
  operator vec3() const;
  
  void apply_impulse(vec3 impulse);
  vec3 momentum() const;
  bool movable() const;
  void timestep(float deltatime);
};


// Class that represents a rectangular prism, with a position, minimum, and maximum point.
// Immutable class
class Hitbox { public:
  vec3 position;
  vec3 negbox;
  vec3 posbox;
  quat rotation;
  
  Hitbox(vec3 pos, vec3 nbox, vec3 pbox, quat rot = quat(1,0,0,0));
  Hitbox();
  
  void points(vec3 points[8]) const;
  void edges(Line edges[12]) const;
  void faces(Plane faces[6]) const;
  // global neg and pos points
  vec3 point1() const;
  vec3 point2() const;
  
  vec3 size() const;
  vec3 local_center() const;
  vec3 global_center() const;
  vec3 local_midpoint() const;
  vec3 global_midpoint() const;
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
  
  Movingpoint transform_in(Movingpoint point) const;
  Hitbox transform_in(Hitbox box) const;
  Movingbox transform_in(Movingbox box) const;
  Movingpoint transform_out(Movingpoint point) const;
  Hitbox transform_out(Hitbox box) const;
  Movingbox transform_out(Movingbox box) const;
  
  // boxes collide when they overlap, so bordering cubes are not colliding
  float axis_projection(vec3 axis) const;
  bool collide(Hitbox other) const;
  vec3 collision(Line line) const;
  
  bool contains_noedge(vec3 point) const;
  bool contains(vec3 point) const;
  // to be contained, all the points in the other box cant be outside the box
  // but they can be on the border.
  bool contains(Hitbox other) const;
  
  void move(vec3 amount);
  void move(vec3 point1, vec3 amount1, vec3 point2, vec3 amount2);
  void move(vec3 point1, vec3 amount1, vec3 point2, vec3 amount2, vec3 point3, vec3 amount3);
  void change_center(vec3 newcenter);
  void dampen(float posthresh, float angthresh);
  
  friend ostream& operator<<(ostream& ofile, const Hitbox& hitbox);
  
  static Hitbox boundingbox_points(vec3 pos, vec3* points, int num);
};



class Movingbox : public Hitbox { public:
  vec3 velocity;
  vec3 angularvel;
  float mass;
  glm::mat3 inertia;
  
  Movingbox();
  Movingbox(Hitbox box, float mass);
  Movingbox(Hitbox box, vec3 vel = vec3(0,0,0), vec3 angvel = vec3(0,0,0), float nmass = 1);
  Movingbox(Hitbox box, vec3 vel, vec3 angvel, float nmass, glm::mat3 inert);
  
  void calc_inertia();
  
  using Hitbox::points;
  void points(Movingpoint points[8]) const;
  
  using Hitbox::transform_in;
  using Hitbox::transform_out;
  Movingpoint transform_in(Movingpoint point) const;
  Movingbox transform_in(Movingbox box) const;
  Movingpoint transform_out(Movingpoint point) const;
  Movingbox transform_out(Movingbox box) const;
  
  glm::mat3 global_inertia() const;
  
  vec3 impulse_at(vec3 globalpos) const;
  void apply_impulse(vec3 impulse);
  void apply_impulse(vec3 impulse, vec3 point);
  
  bool movable() const;
  void timestep(float deltatime);
  void dampen(float posthresh, float angthresh, float velthresh);
};

class PhysicsBody { public:
  FreeBlock* freeblock;
  
  PhysicsBody(FreeBlock* free);
  virtual ~PhysicsBody() {};
  virtual void update() {};
  
	virtual void apply_impulse(vec3 impulse) {};
	virtual void apply_impulse(vec3 impulse, vec3 point) {};
	virtual void apply_impulse(Movingpoint point);
};

class PhysicsBox { public:
  Pixel* pixel = nullptr;
  
  PhysicsBox(Pixel* pix);
  virtual ~PhysicsBox() {};
  virtual void update() {};
};

class PhysicsEngine { public:
  BASE_PLUGIN_HEAD(PhysicsEngine, (World*, float));
  
  World* world;
  float deltatime;
  
  PhysicsEngine(World* nworld, float deltatime);
  
  virtual void tick(float curtime, float deltatime) = 0;
};

class TickRunner { public:
  BASE_PLUGIN_HEAD(TickRunner, (World* world, float deltatime));
  
  World* world;
  float deltatime;
  
  TickRunner(World* nworld, float dt): world(nworld), deltatime(dt) {}
  virtual ~TickRunner() {};
};

#endif
