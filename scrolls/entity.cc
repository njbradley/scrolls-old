#ifndef ENTITY
#define ENTITY

#include "entity.h"

#include "items.h"
#include "world.h"
#include "tiles.h"
#include "blockdata.h"
#include "cross-platform.h"
#include "blocks.h"
#include "blockiter.h"
#include "materials.h"
#include "debug.h"
#include "graphics.h"
#include "libraries.h"

#include <algorithm>

using namespace glm;

using namespace std;

void print(vec3 v) {
    cout << v.x << ' ' << v.y << ' ' << v.z << endl;
}

#define PI 3.14159265358979323846f

float dampen(float value, float thresh) {
  if (std::abs(value) <= thresh) {
    return 0;
  }
  return value;
}

float dampen_to_int(float value, float thresh) {
  float rounded = std::round(value);
  if (std::abs(value - rounded) <= thresh) {
    return rounded;
  }
  return value;
}

vec3 dampen(vec3 value, float thresh) {
  return vec3(dampen(value.x, thresh), dampen(value.y, thresh), dampen(value.z, thresh));
}

vec3 dampen_to_int(vec3 value, float thresh) {
  vec3 rounded = glm::round(value);
  return rounded + dampen(value - rounded, thresh);
}

quat dampen(quat value, float thresh) {
  return quat(dampen_to_int(value.w, thresh), dampen_to_int(value.x, thresh), dampen_to_int(value.y, thresh), dampen_to_int(value.z, thresh));
  // vec3 axis = glm::axis(value);
  // if (axis.x
  // float angle = glm::angle(value);
  
  // return glm::angleAxis(dampen_to_int(angle/PI, thresh)*PI, dampen(axis, thresh));
}

glm::mat3 rotate_tensor(glm::mat3 mat, quat rot) {
  glm::mat3 rotmat = glm::mat3_cast(rot);
  return rotmat * mat * glm::transpose(rotmat);
}


Line::Line(): position(0,0,0), direction(0,0,0) {
  
}

Line::Line(vec3 pos, vec3 dir): position(pos), direction(dir) {
  
}

bool Line::contains(vec3 point) const {
  float dot = glm::dot(direction, point - position);
  return dot == -1 or dot == 1;
}

bool Line::collides(Line other) const {
  return !isnan(collision(other).x);
}

vec3 Line::collision(Line other) const {
  // equasion: dir * time + pos = other.dir * time + other.pos
  // simplifies to: time = other.pos - pos / (dir - other.dir)
  
  vec3 top = other.position - position;
  vec3 bottom = other.direction - direction;
  vec3 result = top / bottom;
  float time;
  bool set = false;
  
  
  for (int i = 0; i < 3; i ++) {
    if (!set) time = result[i];
    if ((bottom[i] == 0 and top[i] != 0) or (bottom[i] != 0 and result[i] != time)) {
      return vec3(nanf(""), 0, 0);
    }
    set = true;
  }
  // cout << top << ' ' << bottom << ' ' << result << ' ' << time << endl;
  return position + direction * time;
}
    

Line::operator bool() const {
  return direction == vec3(0,0,0);
}

Line Line::from_points(vec3 point1, vec3 point2) {
  return Line(point1, (point2 - point1) / glm::length(point2 - point1));
}






Plane::Plane(): position(0,0,0), normal(0,0,0) {
  
}
  
Plane::Plane(vec3 pos, vec3 norm): position(pos), normal(norm) {
  
}

bool Plane::contains(vec3 point) const {
  return glm::dot(position - point, normal) == 0;
}

bool Plane::collides(Line line) const {
  return !isnan(collision(line).x);
}

vec3 Plane::collision(Line line) const {
  /// to get plane equasion: point dot direction == position dot direction;
  float top = glm::dot(normal, position - line.position);
  float bottom = glm::dot(normal, line.direction);
  if (bottom == 0) {
    if (top == 0) {
      return line.position;
    } else {
      return vec3(nanf(""), 0, 0);
    }
  }
  return line.position + (top/bottom) * line.direction;
}

bool Plane::collides(Plane other) const {
  return collision(other);
}

Line Plane::collision(Plane other) const {
  vec3 newdir = glm::cross(normal, other.normal);
  vec3 toother = other.position - position;
  Line topoint (position, toother - normal * glm::dot(toother, normal));
  Line other_topoint (other.position, -toother - other.normal * glm::dot(-toother, other.normal));
  vec3 newpoint = topoint.collision(other_topoint);
  if (isnan(newpoint.x)) {
    newdir = vec3(0,0,0);
  }
  Line result (newpoint, newdir);
  return result;
}

Plane::operator bool() const {
  return normal == vec3(0,0,0);
}

Plane Plane::from_points(vec3 point1, vec3 point2, vec3 point3) {
  vec3 norm = glm::cross(point2 - point1, point3 - point2);
  return Plane(point1, norm/glm::length(norm));
}




Movingpoint::Movingpoint(): Movingpoint(vec3(0,0,0), vec3(0,0,0), 0) {
  
}

Movingpoint::Movingpoint(vec3 pos, float nmass): position(pos), velocity(0,0,0), mass(nmass) {
  
}

Movingpoint::Movingpoint(vec3 pos, vec3 vel, float nmass): position(pos), velocity(vel), mass(nmass) {
  
}

Movingpoint::operator vec3() const {
  return position;
}

vec3 Movingpoint::momentum() const {
  if (!movable()) {
    return vec3(0,0,0);
  }
  return velocity * mass;
}

bool Movingpoint::movable() const {
  return !isinf(mass);
}

void Movingpoint::apply_impulse(vec3 impulse) {
  velocity += impulse / mass;
}

void Movingpoint::timestep(float deltatime) {
  position += velocity * deltatime;
}











Hitbox::Hitbox(vec3 pos, vec3 nbox, vec3 pbox, quat rot):
position(pos), negbox(nbox), posbox(pbox), rotation(rot) {
  
}

Hitbox::Hitbox(): Hitbox(vec3(0,0,0), vec3(0,0,0), vec3(0,0,0)) {
  
}

void Hitbox::points(vec3 points[8]) const {
  for (int i = 0; i < 8; i ++) {
    points[i] = transform_out(size() * vec3(i/4,i/2%2,i%2));
  }
}

void Hitbox::edges(Line edges[12]) const {
  for (int axis = 0; axis < 3; axis ++) {
    vec3 dir (0,0,0);
    vec3 pos (0,0,0);
    dir[axis] = 1;
    vec3 globdir = transform_out_dir(dir);
    for (int i = 0; i < 2; i ++) {
      for (int j = 0; j < 2; j ++) {
        pos[(axis+1)%3] = i*size()[(axis+1)%3];
        pos[(axis+2)%3] = j*size()[(axis+2)%3];
        edges[axis*4+i*2+j] = Line(transform_out(pos), globdir);
      }
    }
  }
}

void Hitbox::faces(Plane faces[6]) const {
  for (int axis = 0; axis < 3; axis ++) {
    vec3 dir (0,0,0);
    dir[axis] = 1;
    faces[axis*2+0] = Plane(transform_out(size()), transform_out_dir(dir));
    faces[axis*2+1] = Plane(transform_out(vec3(0,0,0)), transform_out_dir(-dir));
  }
}

Hitbox Hitbox::boundingbox() const {
  vec3 pointarr[8];
  points(pointarr);
  return boundingbox_points(position, pointarr, 8);
}

vec3 Hitbox::size() const {
  return posbox - negbox;
}

vec3 Hitbox::local_midpoint() const {
  return size() / 2.0f;
}

vec3 Hitbox::global_midpoint() const {
  return transform_out(local_midpoint());
}

vec3 Hitbox::local_center() const {
  return -negbox;
}

vec3 Hitbox::global_center() const {
  return transform_out(local_center());
}

vec3 Hitbox::point1() const {
  return transform_out(vec3(0,0,0));
}

vec3 Hitbox::point2() const {
  return transform_out(size());
}

vec3 Hitbox::dirx() const {
  return transform_out_dir(vec3(1,0,0));
}

vec3 Hitbox::diry() const {
  return transform_out_dir(vec3(0,1,0));
}

vec3 Hitbox::dirz() const {
  return transform_out_dir(vec3(0,0,1));
}

vec3 Hitbox::in_bounds(vec3 pos) const {
  return glm::min(glm::max(pos, vec3(0,0,0)), size());
}

vec3 Hitbox::transform_in(vec3 pos) const {
  return glm::inverse(rotation) * (pos - position) - negbox;
}

vec3 Hitbox::transform_in_dir(vec3 dir) const {
  return glm::inverse(rotation) * dir;
}

quat Hitbox::transform_in(quat rot) const {
  return glm::inverse(rotation) * rot;
}

vec3 Hitbox::transform_out(vec3 pos) const {
  return rotation * (pos + negbox) + position;
}

vec3 Hitbox::transform_out_dir(vec3 dir) const {
  return rotation * dir;
}

quat Hitbox::transform_out(quat rot) const {
  return rotation * rot;
}

Movingpoint Hitbox::transform_in(Movingpoint point) const {
  return Movingpoint(transform_in(point.position), transform_in_dir(point.velocity), point.mass);
}

Hitbox Hitbox::transform_in(Hitbox box) const {
  return Hitbox(
    transform_in(box.position),
    box.negbox,
    box.posbox,
    transform_in(box.rotation)
  );
}

Movingbox Hitbox::transform_in(Movingbox box) const {
  return Movingbox(
    transform_in(Hitbox(box)),
    transform_in_dir(box.velocity),
    transform_in_dir(box.angularvel),
    box.mass,
    rotate_tensor(box.inertia, glm::inverse(rotation))
  );
}

Movingpoint Hitbox::transform_out(Movingpoint point) const {
  return Movingpoint(transform_out(point.position), transform_out_dir(point.velocity), point.mass);
}

Hitbox Hitbox::transform_out(Hitbox box) const {
  return Hitbox(
    transform_out(box.position),
    box.negbox,
    box.posbox,
    transform_out(box.rotation)
  );
}

Movingbox Hitbox::transform_out(Movingbox box) const {
  return Movingbox(
    transform_out(Hitbox(box)),
    transform_out_dir(box.velocity),
    transform_out_dir(box.angularvel),
    box.mass,
    rotate_tensor(box.inertia, rotation)
  );
}

float Hitbox::axis_projection(vec3 axis) const {
  return std::abs(glm::dot(transform_out_dir(size() * vec3(1,0,0)), axis))
       + std::abs(glm::dot(transform_out_dir(size() * vec3(0,1,0)), axis))
       + std::abs(glm::dot(transform_out_dir(size() * vec3(0,0,1)), axis));
}

bool Hitbox::collide(Hitbox other) const {
  /// using separating axis theorem:
  // https://www.jkh.me/files/tutorials/Separating%20Axis%20Theorem%20for%20Oriented%20Bounding%20Boxes.pdf
  vec3 axes[] = {
    dirx(), diry(), dirz(), other.dirx(), other.diry(), other.dirz(),
    glm::cross(dirx(), other.dirx()), glm::cross(dirx(), other.diry()), glm::cross(dirx(), other.dirz()),
    glm::cross(diry(), other.dirx()), glm::cross(diry(), other.diry()), glm::cross(diry(), other.dirz()),
    glm::cross(dirz(), other.dirx()), glm::cross(dirz(), other.diry()), glm::cross(dirz(), other.dirz()),
  };
  
  vec3 tvec = global_midpoint() - other.global_midpoint(); // vec to other box
  if (glm::length(tvec) >= (glm::length(size()) + glm::length(other.size()))/2.0f) {
    return false;
  }
  if (glm::length(tvec) < (std::min(std::min(size().x, size().y), size().z)
      + std::min(std::min(other.size().x, other.size().y), other.size().z)) / 2.0f) {
    return true;
  }
  // cout << glm::length(tvec) << " LEN " << tvec << ' ' << size() << ' ' << other.size() << endl;
  
  for (vec3 axis : axes) {
    if (glm::length(axis) > 0) {
      axis = axis / glm::length(axis);
      // debuglines->render(global_midpoint(), global_midpoint()+axis, vec3(0,0.7f,0.4f));
      
      float aproj = axis_projection(axis);
      float bproj = other.axis_projection(axis);
      float tproj = std::abs(glm::dot(tvec, axis));
      
      float space = tproj - (aproj + bproj) / 2.0f;
      
      // cout << ' ' << space << ' ' << tvec << ' ' << tproj << ' ' << aproj << ' ' << bproj << ' ' << axis << endl;
      // debuglines->render(global_midpoint(), global_midpoint()+axis*space, vec3(1,0,0));
      if (space >= 0) { // touching is not colliding
        return false;
      }
    }
  }
  
  return true;
}

vec3 Hitbox::collision(Line line) const {
  vec3 result (nanf(""), 0, 0);
  bool set = false;
  float score;
  
  cout << " BOUNDS " << transform_out(vec3(0,0,0)) << ' ' << transform_out(size()) << endl;
  
  for (int i = 0; i < 3; i ++) {
    vec3 poses[2] = {
      Plane(transform_out(vec3(0,0,0)), transform_out_dir(dir_array[i+3])).collision(line),
      Plane(transform_out(size()), transform_out_dir(dir_array[i])).collision(line),
    };
    for (vec3 colpos : poses) {
      cout << "  colpos " << colpos << ' ' << transform_in(colpos) << endl;
      if (!isnan(colpos.x) and contains(colpos)) {
        float newscore = glm::dot(colpos - line.position, line.direction);
        cout << "  score " << newscore << endl;
        if (!set or newscore < score) {
          result = colpos;
          score = newscore;
          set = true;
        }
      }
    }
  }
  return result;
}

bool Hitbox::contains_noedge(vec3 point) const {
  point = transform_in(point);
  return 0 < point.x and point.x < size().x
     and 0 < point.y and point.y < size().y
     and 0 < point.z and point.z < size().z;
}

bool Hitbox::contains(vec3 point) const {
  point = transform_in(point);
  // cout << "    contains " << point << endl;
  return 0 <= point.x and point.x <= size().x
     and 0 <= point.y and point.y <= size().y
     and 0 <= point.z and point.z <= size().z;
}

bool Hitbox::contains(Hitbox other) const {
  vec3 otherpoints[8];
  other.points(otherpoints);
  
  for (int i = 0; i < 8; i ++) {
    if (!contains(otherpoints[i])) {
      return false;
    }
  }
  
  return true;
}

void Hitbox::move(vec3 amount) {
  position += amount;
}

void Hitbox::change_center(vec3 newcenter) {
  vec3 oldsize = size();
  position = transform_out(newcenter);
  negbox = -newcenter;
  posbox = oldsize - newcenter;
}

void Hitbox::dampen(float posthresh, float angthresh) {
  position = dampen_to_int(position, posthresh);
  rotation = ::dampen(rotation, angthresh);
}

ostream& operator<<(ostream& ofile, const Hitbox& hitbox) {
  ofile << "Hitbox(pos=" << hitbox.position << " negbox=" << hitbox.negbox << " posbox=" << hitbox.posbox;
  if (hitbox.rotation != quat(1,0,0,0)) {
    ofile << " rot=" << hitbox.rotation;
  }
  ofile << ") ";
  return ofile;
}

Hitbox Hitbox::boundingbox_points(vec3 position, vec3* points, int num) {
  vec3 min = points[0] - position;
  vec3 max = points[0] - position;
  for (int i = 1; i < num; i ++) {
    vec3 relpos = points[i] - position;
    min = glm::min(relpos, min);
    max = glm::max(relpos, max);
  }
  
  return Hitbox(position, min, max);
}




Movingbox::Movingbox(): Hitbox(), velocity(0,0,0), angularvel(0,0,0), mass(1) {
  
}

Movingbox::Movingbox(Hitbox box, float nmass): Hitbox(box), velocity(0,0,0), angularvel(0,0,0), mass(nmass) {
  calc_inertia();
}

Movingbox::Movingbox(Hitbox box, vec3 vel, vec3 angvel, float nmass):
Hitbox(box), velocity(vel), angularvel(angvel), mass(nmass) {
  calc_inertia();
}

Movingbox::Movingbox(Hitbox box, vec3 vel, vec3 angvel, float nmass, glm::mat3 inert):
Hitbox(box), velocity(vel), angularvel(angvel), mass(nmass), inertia(inert) {
  
}

void Movingbox::calc_inertia() {
  if (!movable()) {
    inertia = glm::mat3(0);
    return;
  }
  vec3 dims = size();
  
  vec3 dims2 = 4.0f * size() * size();
  mass = dims.x*dims.y*dims.z;
  float scalar = 1/12.0f;
  inertia = glm::mat3(1.0f);
  inertia[0].x = scalar * mass * (dims2.y + dims2.z);
  inertia[1].y = scalar * mass * (dims2.x + dims2.z);
  inertia[2].z = scalar * mass * (dims2.x + dims2.y);
}

void Movingbox::points(Movingpoint* points) const {
  vec3 center = global_center();
  for (int i = 0; i < 8; i ++) {
    points[i] = transform_out(Movingpoint(size() * vec3(i/4,i/2%2,i%2), mass));
  }
}

// Movingpoint Movingbox::local_midpoint() const {
//   return Movingpoint(size() / 2.0f, vec3(0,0,0), mass);
// }
//
// Movingpoint Movingbox::global_midpoint() const {
//   return transform_out(local_midpoint());
// }
//
// Movingpoint Movingbox::local_center() const {
//   return Movingpoint(-negbox, vec3(0,0,0), mass);
// }
//
// Movingpoint Movingbox::global_center() const {
//   return transform_out(local_center());
// }

Movingpoint Movingbox::transform_in(Movingpoint point) const {
  vec3 newvel = point.velocity - velocity;
  vec3 center_to_pos = point.position - Hitbox::global_center();
  vec3 rotvel = glm::cross(angularvel, center_to_pos);
  return Movingpoint(Hitbox::transform_in(point.position), transform_in_dir(newvel - rotvel), point.mass);
}

Movingbox Movingbox::transform_in(Movingbox box) const {
  vec3 newvel = transform_in(Movingpoint(box.global_center(), 1)).velocity;
  vec3 newangvel = transform_in_dir(box.angularvel - angularvel);
  return Movingbox(
    transform_in(Hitbox(box)),
    newvel,
    newangvel,
    box.mass,
    rotate_tensor(box.inertia, glm::inverse(rotation))
  );
}

Movingpoint Movingbox::transform_out(Movingpoint point) const {
  vec3 newpos = Hitbox::transform_out(point.position);
  vec3 newvel = transform_out_dir(point.velocity) + velocity;
  vec3 center_to_pos = newpos - Hitbox::global_center();
  vec3 rotvel = glm::cross(angularvel, center_to_pos);
  return Movingpoint(newpos, newvel + rotvel, point.mass);
}

Movingbox Movingbox::transform_out(Movingbox box) const {
  vec3 newvel = transform_out(Movingpoint(box.global_center(), 1)).velocity;
  vec3 newangvel = transform_out_dir(box.angularvel) + angularvel;
  return Movingbox(
    transform_out(Hitbox(box)),
    newvel,
    newangvel,
    box.mass,
    rotate_tensor(box.inertia, rotation)
  );
}

glm::mat3 Movingbox::global_inertia() const{
  return rotate_tensor(inertia, rotation);
}

bool Movingbox::movable() const {
  return !isinf(mass);
}


vec3 Movingbox::impulse_at(vec3 globalpos) const {
  if (!movable()) return vec3(0,0,0);
  vec3 impulse = glm::cross(global_inertia() * angularvel, globalpos - global_center());
  // impulse = glm::max(impulse, velocity * mass)
  impulse += velocity * mass;
  return impulse;
}

void Movingbox::apply_impulse(vec3 impulse) {
  velocity += impulse;
}

void Movingbox::apply_impulse(vec3 impulse, vec3 point) {
  // cout << point << endl;
  vec3 centerdir = glm::normalize(global_center() - point);
  vec3 velpart = centerdir * glm::dot(centerdir, impulse);
  vec3 angpart = impulse - velpart;
  velocity += velpart / mass;
  angularvel += glm::inverse(global_inertia()) * glm::cross(point - global_center(), angpart);
  // angularvel += glm::cross(point - global_center(), impulse) / mass;
  debuglines->render(point, global_center(), vec3(0,1,0));
  // debuglines->render(
}

void Movingbox::timestep(float deltatime) {
  position += velocity * deltatime;
  if (angularvel != vec3(0,0,0)) {
    quat drot = glm::angleAxis(glm::length(angularvel) * deltatime, glm::normalize(angularvel));
    rotation = glm::normalize(drot * rotation);
  }
}

void Movingbox::dampen(float posthresh, float angthresh, float velthresh) {
  Hitbox::dampen(posthresh, angthresh);
  velocity = ::dampen(velocity, velthresh);
  angularvel = ::dampen(angularvel, velthresh);
}



MovingboxStep::MovingboxStep(): Movingbox(), unapplied_vel(0,0,0), unapplied_movement(0,0,0) {
  
}

MovingboxStep::MovingboxStep(Movingbox box): Movingbox(box), unapplied_vel(0,0,0), unapplied_movement(0,0,0) {
  
}

void MovingboxStep::apply_impulse(vec3 impulse) {
  vec3 newvel = impulse / mass;
  
  if (glm::length(newvel) > glm::length(unapplied_vel)) {
    vec3 tmp = unapplied_vel;
    unapplied_vel = newvel;
    newvel = tmp;
  }
  vec3 vel_norm = glm::normalize(unapplied_vel);
  if (glm::dot(vel_norm, newvel) >= 0) {
    unapplied_vel += newvel - vel_norm * glm::dot(vel_norm, newvel);
  } else {
    unapplied_vel += newvel;
  }
}

void MovingboxStep::apply_impulse(vec3 impulse, vec3 point) {
  vec3 newvel = impulse / mass;
  vec3 newangvel = glm::inverse(global_inertia()) * glm::cross(point - global_center(), impulse);
  
  debuglines->render(point, point+impulse, vec3(0,1,1));
  
  apply_impulse(impulse);
  
  unapplied_angvel.push_back(newangvel);
}

void MovingboxStep::move(vec3 amount) {
  if (glm::length(amount) > glm::length(unapplied_movement)) {
    vec3 tmp = unapplied_movement;
    unapplied_movement = amount;
    amount = tmp;
  }
  vec3 mov_norm = glm::normalize(unapplied_movement);
  if (glm::dot(mov_norm, amount) >= 0) {
    unapplied_movement += amount - mov_norm * glm::dot(mov_norm, amount);
  } else {
    unapplied_movement += amount;
  }
}

void MovingboxStep::apply_forces() {
  velocity += unapplied_vel;
  position += unapplied_movement;
  
  if (unapplied_angvel.size() == 0) return;
  
  vec3 newangvel (0,0,0);
  
  for (int i = 0; i < unapplied_angvel.size(); i ++) {
    if (glm::length(unapplied_angvel[i]) < 0.01f) {
      newangvel = vec3(0,0,0);
      break;
    }
    vec3 angveldir = glm::normalize(unapplied_angvel[i]);
    debuglines->render(global_center(), global_center() + angveldir);
    bool matched = false;
    for (int j = 0; j < i; j ++) {
      if (glm::dot(glm::normalize(unapplied_angvel[j]), angveldir) < -0.99f) {
        matched = true;
      }
    }
    if (!matched) {
      newangvel += unapplied_angvel[i];
    }
  }
  
  newangvel /= unapplied_angvel.size();
  angularvel += newangvel;
  
  unapplied_vel = vec3(0,0,0);
  unapplied_angvel.clear();
  unapplied_movement = vec3(0,0,0);
}





CollisionManifold::CollisionManifold(): result(false) {
  
}

CollisionManifold::CollisionManifold(Movingbox* nbox1, Movingbox* nbox2, MovingboxStep* ndest1, MovingboxStep* ndest2):
box1(nbox1), box2(nbox2), dest1(ndest1), dest2(ndest2) {
  result = collision_result();
}
  

bool CollisionManifold::collision_result() {
  /// using separating axis theorem:
  // https://www.jkh.me/files/tutorials/Separating%20Axis%20Theorem%20for%20Oriented%20Bounding%20Boxes.pdf
  vec3 axes[] = {
    box1->dirx(), box1->diry(), box1->dirz(), box2->dirx(), box2->diry(), box2->dirz(),
    glm::cross(box1->dirx(), box2->dirx()), glm::cross(box1->dirx(), box2->diry()), glm::cross(box1->dirx(), box2->dirz()),
    glm::cross(box1->diry(), box2->dirx()), glm::cross(box1->diry(), box2->diry()), glm::cross(box1->diry(), box2->dirz()),
    glm::cross(box1->dirz(), box2->dirx()), glm::cross(box1->dirz(), box2->diry()), glm::cross(box1->dirz(), box2->dirz()),
  };
  
  vec3 tvec = box1->global_midpoint() - box2->global_midpoint(); // vec to box2 box
  if (glm::length(tvec) >= (glm::length(box1->size()) + glm::length(box2->size()))/2.0f) {
    return false;
  }
  // if (glm::length(tvec) < (std::min(std::min(box1->size().x, box1->size().y), box1->size().z)
  //     + std::min(std::min(box2->size().x, box2->size().y), box2->size().z)) / 2.0f) {
  //   return true;
  // }
  
  vec3 least_axis;
  float least_overlap = -9999999;
  vec3 least_point;
  float my_proj = 0;
  float other_proj = 0;
  float t_proj;
  
  for (vec3 axis : axes) {
    if (glm::length(axis) > 0) {
      axis = axis / glm::length(axis);
      
      float aproj = box1->axis_projection(axis);
      float bproj = box2->axis_projection(axis);
      float tproj = std::abs(glm::dot(tvec, axis));
      
      float space = tproj - (aproj + bproj) / 2.0f;
      
      if (space >= 0) { // touching is not colliding
        return false;
      }
      if (space > least_overlap) {
        least_overlap = space;
        least_axis = axis;
        my_proj = aproj;
        other_proj = bproj;
        t_proj = tproj;
      }
    }
  }
  
  if (glm::dot(tvec, least_axis) > 0) {
    col_axis = least_axis;
  } else {
    col_axis = -least_axis;
  }
  col_amount = -least_overlap;
  
  
  vector<vec3> points;
  
  vec3 tmppoints[8];
  box1->points(tmppoints);
  for (vec3 point : tmppoints) {
    if (box1->contains(point) and box2->contains(point)) {
      points.push_back(point);
    }
  }
  
  // box2->points(tmppoints);
  // for (vec3 point : tmppoints) {
  //   if (box1->contains(point) and box2->contains(point)) {
  //     points.push_back(point);
  //   }
  // }
  
  Line edges[12];
  Plane faces[6];
  box1->edges(edges);
  box2->faces(faces);
  for (Line edge : edges) {
    for (Plane face : faces) {
      vec3 col = face.collision(edge);
      if (!std::isnan(col.x) and box1->contains(col) and box2->contains(col)) {
        points.push_back(col);
      }
    }
  }
  
  // box2->edges(edges);
  // box1->faces(faces);
  // for (Line edge : edges) {
  //   for (Plane face : faces) {
  //     vec3 col = face.collision(edge);
  //     if (!std::isnan(col.x) and box1->contains(col) and box2->contains(col)) {
  //       points.push_back(col);
  //     }
  //   }
  // }
  
  for (vec3 point : points) {
    vec3 impulse = box1->impulse_at(point) - box2->impulse_at(point);
    Movingpoint point1 (point, impulse, 1);
    // Movingpoint point1 = box1->transform_out(Movingpoint(box1->transform_in(point), box1->mass));
    // Movingpoint point2 = box2->transform_out(Movingpoint(box2->transform_in(point), box2->mass));
    // point1.velocity = point1.momentum() - point2.momentum();
    // point1.mass = 1;
    debuglines->render(point1.position + vec3(-0.1,0,0), point1.position + vec3(0.1,0,0));
    debuglines->render(point1.position + vec3(0,0,-0.1), point1.position + vec3(0,0,0.1));
    col_points.push_back(point1);
  }
  
  /*
  vec3 close_points[4];
  for (int i = 0, j = 0; i < 8; i ++) {
    vec3 inner_point = box1->size() * vec3(i/4,i/2%2,i%2) - box1->size() / 2.0f;
    vec3 point = box1->transform_out_dir(inner_point);
    if (glm::dot(point, tvec) < 0) {
      close_points[j++] = point;
    }
  }
  
  vec3 other_close_points[4];
  for (int i = 0, j = 0; i < 8; i ++) {
    vec3 inner_point = box2->size() * vec3(i/4,i/2%2,i%2) - box2->size() / 2.0f;
    vec3 point = box2->transform_out_dir(inner_point);
    if (glm::dot(point, tvec) > 0) {
      other_close_points[j++] = point;
    }
  }
  
  float max_proj = 0;
  vec3 allpoints (0,0,0);
  int num_points = 0;
  
  vector<Movingpoint> contained_points;
  vector<Movingpoint> past_points;
  
  
  for (vec3 point : close_points) {
    float proj = std::abs(glm::dot(point, least_axis));
    Movingpoint real_point = box1->transform_out(Movingpoint(box1->size()/2.0f + box1->transform_in_dir(point), box1->mass));
    Movingpoint other_point = box2->transform_out(Movingpoint(box2->transform_in(real_point.position), box2->mass));
    real_point.velocity = real_point.momentum() - other_point.momentum();
    real_point.mass = 1;
    if (proj >= t_proj - other_proj/2) {
      past_points.push_back(real_point);
    }
    if (box2->contains(real_point)) {
      contained_points.push_back(real_point);
    }
  }
  
  for (vec3 point : other_close_points) {
    float proj = std::abs(glm::dot(point, least_axis));
    Movingpoint real_point = box2->transform_out(Movingpoint(box2->size()/2.0f + box2->transform_in_dir(point), box2->mass));
    Movingpoint other_point = box1->transform_out(Movingpoint(box1->transform_in(real_point.position), box1->mass));
    real_point.velocity = other_point.momentum() - real_point.momentum();
    real_point.mass = 1;
    if (proj >= t_proj - my_proj/2) {
      past_points.push_back(real_point);
    }
    if (box1->contains(real_point)) {
      contained_points.push_back(real_point);
    }
  }
  
  if (contained_points.size() != 0) {
    col_points.insert(col_points.begin(), contained_points.begin(), contained_points.end());
    // cout << "colpoint contained " << col_point << endl;
  } else {
    cout << "BADDDDDD entity.cc:674" << endl;
    // vec3 col_point = past_points_sum / (float)past_points.size();
    // col_points.push_back(col_point);
    // col_points.insert(col_points.begin(), past_points.begin(), past_points.end());
    // cout << "colpoint past " << col_point << endl;
  }*/
  
  return true;
}

CollisionManifold::operator bool() const {
  return result;
}

void CollisionManifold::move_boxes() {
  float massratio = box1->mass / (box1->mass + box2->mass);
  if (!box1->movable()) {
    massratio = 1;
  }
  if (!box2->movable()) {
    massratio = 0;
  }
  
  dest1->move(col_axis * col_amount * (1-massratio));
  dest2->move(-col_axis * col_amount * massratio);
}
  

void CollisionManifold::apply_forces() {
  
  for (Movingpoint point : col_points) {
    // cout << " point " << point.position << ' ' << point.velocity << endl;
    if (glm::dot(point.velocity, col_axis) < 0) {
      vec3 vel = col_axis * glm::dot(point.velocity, col_axis);
      // vel += -col_axis;
      // vel *= 1.0f/col_points.size();
      // vel *= 0.3f;
      // cout << "   " << point.velocity << endl;
      dest1->apply_impulse(-vel, point.position);
      dest2->apply_impulse(vel, point.position);
      // cout << "  box " << box1->velocity << ' ' << box1->angularvel << endl;
      debuglines->render(point.position, point.position + vel, vec3(1,1,0));
    }
  }
}

void CollisionManifold::apply_friction() {
  
  for (Movingpoint point : col_points) {
    // friction
    vec3 vel = point.velocity - col_axis * glm::dot(point.velocity, col_axis);
    // vel *= 1.0f/col_points.size();
    vel *= 0.1f;
    dest1->apply_impulse(-vel, point.position);
    dest2->apply_impulse(vel, point.position);
  }
}


DEFINE_PLUGIN(PhysicsEngine);

PhysicsEngine::PhysicsEngine(World* nworld, float dt): world(nworld), deltatime(dt) {
  
}

/*
vec3 CollisionManifold::torque_point(vec3 mass_point, vec3 vel_dir) const {
  if (!result) return vec3(0,0,0);
  
  if (col_points.size() == 1) {
    return col_points[0];
  }
  
  Line vel_line (mass_point, vel_dir);
  
  int num_points = col_points.size();
  float av_proj = 0;
  // finding average projection to vel_line
  for (vec3 point : col_points) {
    // cout << " col_point " << point << endl;
    float proj = glm::dot(point, col_axis);
    av_proj += proj;
  }
  av_proj /= num_points;
  
  Plane col_plane (col_axis * av_proj, col_axis);
  vec3 start_colpos = col_plane.collision(vel_line);
  
  debuglines->render(mass_point, start_colpos, vec3(1,0,1));
  
  vec3 newpoints[num_points+1];
  vec3 center (0,0,0);
  
  int num_removed = 0;
  // flattening all points onto the plane (and remove duplicates)
  for (int i = 0; i < num_points; i ++) {
    vec3 relpoint = col_points[i] - col_plane.position;
    relpoint = relpoint - col_axis * glm::dot(col_axis, relpoint);
    vec3 newpoint = relpoint + col_plane.position;
    // cout << " new point " <<  newpoint << " changed: " << col_points[i] - newpoint << endl;
    
    bool unique = true;
    for (int j = 0; j < i - num_removed; j ++) {
      unique = unique and newpoints[j] != newpoint;
    }
    
    if (i == 0 or unique) {
      newpoints[i-num_removed] = newpoint;
      center += newpoint;
    } else {
      num_removed ++;
    }
  }
  num_points -= num_removed;
  center /= float(num_points);
  // cout << "Num points " << num_points << endl;
  
  if (num_points == 1) {
    return newpoints[0];
  }
  
  // reordering into list of line segments
  for (int i = 1; i < num_points; i ++) {
    vec3 lastpoint = newpoints[i-1];
    float curscore = atan2(glm::dot(glm::cross(lastpoint-center, newpoints[i]-center), col_axis), glm::dot(lastpoint-center, newpoints[i]-center));
    for (int j = i; j < num_points; j ++) {
      float score = atan2(glm::dot(glm::cross(lastpoint-center, newpoints[j]-center), col_axis), glm::dot(lastpoint-center, newpoints[j]-center));
      if ((curscore < 0 or score < curscore) and score > 0) {
        vec3 tmp = newpoints[i];
        newpoints[i] = newpoints[j];
        newpoints[j] = tmp;
        curscore = score;
      }
    }
  }
  // adding the first point at the end so that all line segments are there
  if (newpoints[0] != newpoints[num_points-1]) {
    newpoints[num_points] = newpoints[0];
    num_points ++;
  }
  
  int closest_point_index = -1;
  float closest_point_dist;
  bool outside = false;
  // find closest point and check if start_colpos is inside
  for (int i = 1; i < num_points; i ++) {
    vec3 lastpoint = newpoints[i-1];
    vec3 curpoint = newpoints[i];
    
    vec3 out_dir = glm::normalize(glm::cross(lastpoint-curpoint, col_axis));
    
    debuglines->render(lastpoint - vec3(0,0.1f,0), curpoint - vec3(0,0.1f,0), vec3(0,1,0.5f));
    debuglines->render((lastpoint+curpoint)/2.0f - vec3(0,0.1f,0),
          (lastpoint+curpoint)/2.0f - vec3(0,0.1f,0) + out_dir, vec3(0,1,1));
    
    vec3 dir_from_line = start_colpos - lastpoint;
    if (glm::dot(dir_from_line, out_dir) < 0) {
      outside = true;
    }
    
    float dist = glm::length(curpoint - start_colpos);
    if (closest_point_index == -1 or dist < closest_point_dist) {
      closest_point_index = i;
      closest_point_dist = dist;
    }
  }
  
  if (outside) {
    vec3 closest_point = newpoints[closest_point_index];
    vec3 lower_point = newpoints[closest_point_index-1];
    vec3 upper_point = newpoints[(closest_point_index + 1) % num_points];
    
    vec3 lower_line_dir = glm::normalize(lower_point - closest_point);
    // lower_line_dir /= glm::length(lower_line_dir);
    vec3 upper_line_dir = glm::normalize(upper_point - closest_point);
    // upper_line_dir /= glm::length(upper_line_dir);
    
    float lower_dot = glm::dot(start_colpos - closest_point, lower_line_dir);
    float upper_dot = glm::dot(start_colpos - closest_point, upper_line_dir);
    
    if (lower_dot > 0) {
      start_colpos = lower_line_dir * lower_dot + closest_point;
    } else if (upper_dot > 0) {
      start_colpos = upper_line_dir * upper_dot + closest_point;
    } else {
      start_colpos = closest_point;
    }
  }
  debuglines->render(mass_point, start_colpos, vec3(1,0.5f,1));
  
  return start_colpos;
}*/

/*
void CollisionManifold::combine(CollisionManifold other) {
  if (!result) {
    *this = other;
  } else if (col_axis == other.col_axis) {
    col_amount = std::max(col_amount, other.col_amount);
    col_points.insert(col_points.end(), other.col_points.begin(), other.col_points.end());
  } else {
    cout << "ERR: combine called with not same col_axis" << endl;
  }
}



CollisionPlan::CollisionPlan(Hitbox nbox): box(nbox) {
  
}

void CollisionPlan::add_box(Hitbox newbox) {
  CollisionManifold manifold(box, newbox);
  if (manifold) {
    collisions[manifold.col_axis].combine(manifold);
  }
}

Hitbox CollisionPlan::newbox() const {
  if (collisions.size() == 0) {
    return box;
  }
  
  vec3 movementdir (0,0,0);
  for (const std::pair<vec3,CollisionManifold>& kv : collisions) {
    movementdir += kv.second.col_amount * kv.second.col_axis;
  }
  
  movementdir = glm::normalize(movementdir);
  float max_dist_needed = 0;
  for (const std::pair<vec3,CollisionManifold>& kv : collisions) {
    vec3 colvec = kv.second.col_amount * kv.second.col_axis;
    float proj = glm::dot(movementdir, kv.second.col_axis);
    if (proj > 0) {
      float dist_needed = 1/proj * kv.second.col_amount;
      if (dist_needed > max_dist_needed) {
        max_dist_needed = dist_needed;
      }
    }
  }
  
  Hitbox newbox = box;
  newbox.move(movementdir * max_dist_needed);
  return newbox;
}

vec3 CollisionPlan::constrain_vel(vec3 velocity) const {
  bool drag = false;
  for (const std::pair<vec3,CollisionManifold>& kv : collisions) {
    if (glm::dot(velocity, kv.second.col_axis) < 0) {
      velocity -= glm::dot(velocity, kv.second.col_axis) * kv.second.col_axis;
      drag = true;
    }
  }
  if (drag) {
    velocity *= 0.95f;
  }
  return velocity;
}

vec3 CollisionPlan::torque(vec3 mass_point, vec3 vel_dir) const {
  vec3 torque (0,0,0);
  
  vec3 vel_dir_norm = vel_dir;
  if (vel_dir != vec3(0,0,0)) {
    vel_dir_norm /= glm::length(vel_dir);
  }
  
  for (const std::pair<vec3,CollisionManifold>& kv : collisions) {
    vec3 torque_point = kv.second.torque_point(mass_point, vel_dir);
    debuglines->render(mass_point, torque_point, vec3(1,0,1));
    float ratio = std::abs(glm::dot(kv.second.col_axis, vel_dir_norm));
    cout << ratio << endl;
    torque += glm::cross(mass_point - torque_point, vel_dir * ratio);
  }
  return torque;
}

vec3 CollisionPlan::constrain_torque(vec3 mass_point, vec3 vel_dir, vec3 torque) const {
  if (collisions.size()) {
    return torque;
  }
  /*
  debuglines->render(mass_point, mass_point + torque, vec3(1,1,0));
  vector<vec3> newtorques;
  vec3 newtorque (0,0,0);
  for (const std::pair<vec3,CollisionManifold>& kv : collisions) {
    for (vec3 colpoint : kv.second.col_points) {
      vec3 vec_to_colpoint = colpoint - mass_point;
      vec3 newtorque = glm::cross(vec_to_colpoint, -vel_dir);
      if (glm::length(newtorque) != 0) {
        newtorque = glm::normalize(newtorque);
        newtorques.push_back(newtorque);
        newtorque_av += newtorque;
        debuglines->render(mass_point, mass_point + newtorque, vec3(0,1,0));
        debuglines->render(mass_point, colpoint);
        debuglines->render(colpoint, colpoint-vel_dir);
      }
    }
  }
  
  if (newtorques.size() == 0) {
    return torque;
  }
  
  newtorque_av = glm::normalize(newtorque_av / newtorques.size());
  
  for (vec3 newtor : newtorques) {
    if (glm::dot(newtor, newtorque_av) <= 0) {
      return vec3(0,0,0);
    }
  }
  
  // if (glm::dot(newtorque_av, torque) <
  
  debuglines->render(mass_point, mass_point + torque, vec3(0,1,1));
  //
  return torque;
}
*/
// MovingHitbox::MovingHitbox(Hitbox box, vec3 vel = vec3(0,0,0), vec3 angvel = vec3(0,0,0), float newmass = 1.0f):
// Hitbox(box), velocity(vel), angularvel(angvel), mass(newmass) {
//
// }
//
// MovingHitbox::MovingHitbox() {
//
// }








//
// ImpactManifold::ImpactManifold(): num_hitpoints(0) {
//
// }
//
// ImpactManifold::ImpactManifold(Hitbox nbox, float maxtime): box1(nbox), time(maxtime), num_hitpoints(0) {
//
// }
//
// ImpactManifold::ImpactManifold(Hitbox nbox1, Hitbox nbox2, float maxtime): box1(nbox1), box2(nbox2) {
//   time = maxtime;
//   num_hitpoints = 0;
//   if (box1.collide(box2, maxtime, &time)) {
//     num_hitpoints = 1;
//   }
// }
//
// ImpactManifold::operator bool() {
//   return num_hitpoints;
// }
//
// Hitbox ImpactManifold::newbox1(float dt) {
//   if (dt < time or num_hitpoints == 0) {
//     return box1.timestep(dt);
//   } else {
//     logger->log(3) << " getting collided box AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << endl;
//     return newbox1().timestep(dt - time);
//   }
// }
//
// Hitbox ImpactManifold::newbox1() {
//   Hitbox newbox = box1.timestep(time);
//   newbox.velocity = vec3(0,0,0);
//   return newbox;
// }
//
// Hitbox ImpactManifold::newbox2(float dt) {
//   if (dt < time or num_hitpoints == 0) {
//     return box2.timestep(dt);
//   } else {
//     return newbox2().timestep(dt - time);
//   }
// }
//
// Hitbox ImpactManifold::newbox2() {
//   Hitbox newbox = box2.timestep(time);
//   newbox.velocity = vec3(0,0,0);
//   return newbox;
// }
//
//
// ImpactPlan::ImpactPlan(): time(0) {
//
// }
//
// ImpactPlan::ImpactPlan(FreeBlock* freeblock, float maxtime): startbox(freeblock->box) {
//   // cout << "Starting impact plan " << startbox << endl;
//   Hitbox box = startbox;
//   time = 0;
//   int i = 0;
//
//   while (time < maxtime) {
//     // cout << " time " << time << endl;
//     ImpactManifold curimpact(box, maxtime - time);
//     // freeblock -> world collisisions
//     FreeBlockIter freeiter (freeblock->highparent, box);
//     for (int i = 0; i < freeiter.num_bases; i ++) {
//       for (Pixel* pix : freeiter.bases[i]->iter()) {
//         if (pix->value != 0) {
//           // cout << " test " << box << ' '  << pix->parbl->hitbox() << endl;
//           ImpactManifold impact(box, pix->parbl->hitbox(), maxtime - time);
//           if (impact and (!curimpact or impact.time < curimpact.time)) {
//             // cout << pix->parbl->hitbox() << ' ' << impact.time << endl;
//             curimpact = impact;
//           }
//         }
//       }
//     }
//     i ++;
//
//     /*
//     // freeblock -> freeblock collisiions
//     for (int x = -1; x < 2; x ++) {
//       for (int y = -1; y < 2; y ++) {
//         for (int z = -1; z < 2; z ++) {
//           Block* block = highparent->get_global(highparent->globalpos + ivec3(x,y,z) * highparent->scale, highparent->scale);
//           if (block != nullptr and world != block->world) {
//
//           }
//           // timestep_freeblock(deltatime, block);
//         }
//       }
//     }*/
//
//     impacts.push_back(curimpact);
//     time += curimpact.time;
//     box = curimpact.newbox1();
//     // if (curimpact) {
//     //   cout << " found impacr" << curimpact.box2 << endl;
//     //   cout << curimpact.box1 << ' ' << curimpact.time << endl;
//     // } else {
//     //   time = maxtime;
//     // }
//   }
// }
//
// Hitbox ImpactPlan::newbox(float dt) {
//   int i = 0;
//   float newtime = dt;
//   while (i < impacts.size() and impacts[i].time < dt) {
//     newtime -= impacts[i].time;
//     i ++;
//   }
//
//   return impacts[i].newbox1(newtime);
// }
//
//
//   // if (impacts.size() == 0) {
//   //   return startbox.timestep(dt);
//   // } else {
//   //   cout << "looking at impacts "<< endl;
//   //   int i = 0;
//   //   float newtime = dt;
//   //   while (i < impacts.size() and impacts[i].time < dt) {
//   //     i ++;
//   //     newtime -= impacts[i].time;
//   //   }
//   //   cout << i << endl;
//   //   cout << impacts[i].box1 << ' ' << newtime << ' ' << impacts[i].time << endl;
//   //   Hitbox nbox = impacts[i].newbox1(newtime);
//   //   if (nbox.position.x > 100000 or nbox.position.x < 0.00001f) {
//   //     cout << "Im huge " << endl;
//   //   }
//   //   return nbox;
//   // }
// // }


Entity::Entity(World* nworld, vec3 pos, vec3 hitbox1, vec3 hitbox2):
world(nworld), position(pos), box1(hitbox1), box2(hitbox2), alive(true), immune(false), flying(false)
{
    angle = vec2(0,0);
    vel = vec3(0,0,0);
    lastTime = getTime();
    for (int i = 0; i < 7; i ++) {
        consts[i] = false;
        old_consts[i] = false;
    }
}

void Entity::timestep() {
        
    // Compute time difference between current and last frame
    float currentTime(getTime());
    deltaTime = (currentTime - lastTime);
    
    on_timestep(deltaTime);
    if (deltaTime > 0.2f) {
        deltaTime = 0.2f;
        cout << "warning: had to drop ticks" << endl;
    }
    
    step_distance -= glm::length(vec3(vel.x, int(vel.y), vel.z))*deltaTime;
  	if (consts[4] and step_distance < 0 and !immune) {
  		step_distance = 4;
  		audio->play_sound("step", position - vec3(0, 4, 0), 0.5);
  	}
    
    if (damage_health > 0.01) {
      double decrement = std::pow(0.99,1/deltaTime);
      health -= damage_health*decrement;
      damage_health -= damage_health*decrement;
      healing_health = 0;
    } else {
      damage_health = 0;
    }
    if (healing_health > 0.01) {
      double increment = std::min(healing_speed * deltaTime, healing_health);
      health += increment;
      healing_health -= increment;
      damage_health = 0;
    } else {
      healing_speed = 0;
      healing_health = 0;
    }
    
    //cout << vel.x << ' ' << vel.y << ' ' << vel.z << endl;
    move(vel*deltaTime, deltaTime);
    if (!spectator) {
        calc_constraints();
    } else {
        vel.y = 0;
    }
    if (!spectator and !flying) {
      if (in_water) {
        vel += vec3(0, -40 * density, 0) * deltaTime;
      } else {
        vel += vec3(0,-40,0) * deltaTime;
      }
    }
    //for (bool b : consts) {
    //    cout << b << ' ';
    //}
    //cout << endl;
    //cout << deltaTime << endl;
    lastTime = currentTime;
}

void Entity::drop_ticks() {
	lastTime = getTime();
}

void Entity::tick() {
  
}

bool Entity::is_solid_block(Block* block) {
  return block != nullptr and (block->pixel->value != 0 and block->pixel->value != 7);
}

void Entity::calc_constraints() {
    //int begin = clock();
    vec3 coords_array[3] = {{0,1,1}, {1,0,1}, {1,1,0}};
    vec3 dir_array[3] =    {{1,0,0}, {0,1,0}, {0,0,1}};
    
    vec3 newpos(position);
    
    bool new_consts[7];
    for (int i = 0; i < 7; i ++) {
      new_consts[i] = false;
    }
    Material* old_mat_consts[6];
    for (int i = 0; i < 6; i ++) {
      //if (!old_consts[i]) {
      old_mat_consts[i] = mat_consts[i];
        mat_consts[i] = nullptr;
      //}
    }
    
    vec3 newbox2 = box2 - 1.0f - axis_gap;
    vec3 newbox1 = box1 + axis_gap;
    
    //box2 = vec3(0,0,0);
    
    //this system works by creating a slice of blocks that define each side of collision. the sides all share either the most positive point or the most negative point
    //the shlice is tested to see if any blocks are int it (2) and if there are blocks, the players location is rounded to outside the block (3)
    //axis gap make sure a side detects the collision of blocks head on before the side detectors sense the blocks. Each slice is moved out from the player by axis_gap
    //cout << "----------------------------------" << endl;
    //print(position);
    
    
    
    //for (Collider* collider : colliders) {
      Collider* collider = world;
      vec3 rel_pos = position - collider->get_position();
      vec3 new_rel_pos = newpos - collider->get_position();
      for (int axis = 0; axis < 3; axis ++) {
        vec3 coords = coords_array[axis];
        vec3 dir = dir_array[axis];
        
        // pos is the positive point of the hitbox rounded up to the nearest block. neg is the negative most point rounded down
        vec3 neg = floor( rel_pos + newbox1 - axis_gap*dir );// + vec3(0,1,0);
        vec3 pos = ceil( rel_pos + newbox2 + axis_gap*dir );
        if (axis != 1 and !consts[2]) {
          neg = floor( rel_pos + newbox1 - axis_gap*dir + vec3(0,1,0));
        }
        
        //positive side
        //pos2 is the box2 point for the face
        vec3 pos2 = pos*dir + neg*coords;
        bool constraint = false;
        for (int x = (int)pos2.x; x < pos.x+1; x ++) {
            for (int y = (int)pos2.y; y < pos.y+1; y ++) {
                for (int z = (int)pos2.z; z < pos.z+1; z ++) {
                    Block* pix = collider->get_global(x,y,z,1);
                    bool new_const = is_solid_block(pix);
                    if (new_const) {
                      Material* mat = blockstorage[pix->pixel->value]->material;
                      if (mat_consts[axis] == nullptr or mat_consts[axis]->toughness < mat->toughness) {
                        mat_consts[axis] = mat;
                      }
                    }
                    constraint = constraint or new_const;
                }
            }
        }
        new_consts[axis] = new_consts[axis] or constraint;
        if (constraint) {
            new_rel_pos[axis] = floor(rel_pos[axis] + newbox2[axis] + axis_gap) - newbox2[axis] - axis_gap;
        }
        vec3 neg2 = neg*dir + pos*coords;
        if (axis == 1) {
          //neg -= vec3(0.5,0,0.5);
          //neg2 += vec3(0.5,0,0.5);
          int max_y = INT_MIN;
          bool max_y_set = false;
          constraint = false;
          for (int x = (int)neg.x; x < neg2.x+1; x ++) {
              for (int y = (int)neg.y; y < neg2.y+1; y ++) {
                  for (int z = (int)neg.z; z < neg2.z+1; z ++) {
                      Block* pix = collider->get_global(x,y,z,1);
                      Material* mat = nullptr;
                      if (!is_solid_block(pix)) {
                        continue;
                      } else {
                        mat = blockstorage[pix->pixel->value]->material;
                      }
                      constraint = true;
                      for (int yoff = 1; yoff < 3; yoff ++) {
                        pix = collider->get_global(x,y+yoff,z,1);
                        if (!is_solid_block(pix)) {
                          if (y+yoff-1 > max_y) {
                            max_y = y+yoff-1;
                            max_y_set = true;
                          }
                          constraint = true;
                          break;
                        } else {
                          mat = blockstorage[pix->pixel->value]->material;
                        }
                      }
                      if (mat != nullptr and (mat_consts[axis+3] == nullptr or mat_consts[axis+3]->toughness < mat->toughness)) {
                        mat_consts[axis+3] = mat;
                      }
                  }
              }
          }
          
          if (!constraint) {
            neg = floor( rel_pos + newbox1 - axis_gap*1.4f );
            pos = ceil( rel_pos + newbox2 + axis_gap*1.4f );
            neg2 = neg*dir + pos*coords;
            
            for (int x = (int)neg.x; x < neg2.x+1; x ++) {
              for (int y = (int)neg.y; y < neg2.y+1; y ++) {
                for (int z = (int)neg.z; z < neg2.z+1; z ++) {
                  Block* floor = collider->get_global(x,y,z,1);
                  if (is_solid_block(floor)) {
                    Block* above = collider->get_global(x,y+1,z,1);
                    if (!is_solid_block(above)) {
                      constraint = true;
                      Material* mat = blockstorage[floor->pixel->value]->material;
                      if (mat_consts[axis+3] == nullptr or mat_consts[axis+3]->toughness < mat->toughness) {
                        mat_consts[axis+3] = mat;
                      }
                    }
                  }
                  
                }
              }
            }
            
            if (constraint) {
              float newpos = ceil(rel_pos[axis] + newbox1[axis] - axis_gap) - newbox1[axis] + axis_gap;
              if (std::abs(newpos - new_rel_pos[axis]) > 0.3) {
                constraint = false;
              } else {
                new_rel_pos[axis] = newpos;
              }
            }
            
          } else {
            if (constraint and max_y_set) {
              double newpos = max_y - newbox1[axis] + axis_gap + 1;
              if (newpos > new_rel_pos[1]) {
                double diff = newpos - new_rel_pos[1];
                if (diff > 0.2) {
                  diff = 0.2;
                }
                new_rel_pos[1] += diff;
              } else {
                new_consts[axis+3] = false;
              }
              //ceil(rel_pos[axis] + newbox1[axis] - axis_gap) - newbox1[axis] + axis_gap;
            }
          }
          
          new_consts[axis+3] = new_consts[axis+3] or constraint;
          
        } else {
          constraint = false;
          for (int x = (int)neg.x; x < neg2.x+1; x ++) {
              for (int y = (int)neg.y; y < neg2.y+1; y ++) {
                  for (int z = (int)neg.z; z < neg2.z+1; z ++) {
                      Block* pix = collider->get_global(x,y,z,1);
                      bool new_const = is_solid_block(pix);
                      if (new_const) {
                        Material* mat = blockstorage[pix->pixel->value]->material;
                        if (mat_consts[axis+3] == nullptr or mat_consts[axis+3]->toughness < mat->toughness) {
                          mat_consts[axis+3] = mat;
                        }
                      }
                      constraint = constraint or new_const;
                  }
              }
          }
          new_consts[axis+3] = new_consts[axis+3] or constraint;
          if (constraint) {
            new_rel_pos[axis] = ceil(rel_pos[axis] + newbox1[axis] - axis_gap) - newbox1[axis] + axis_gap;
          }
        }
      }
      /// torso check: to tell if the constraints are only on the feet
      
      vec3 neg = floor( rel_pos + newbox1 - axis_gap) ;
      vec3 pos = ceil( rel_pos + newbox2 + axis_gap );
      in_water = false;
      for (int x = (int)neg.x; x < pos.x+1 and !in_water; x ++) {
        for (int y = (int)neg.y; y < pos.y+1 and !in_water; y ++) {
          for (int z = (int)neg.z; z < pos.z+1 and !in_water; z ++) {
            Block* pix = collider->get_global(x,y,z,1);
            if (pix != nullptr and pix->pixel->value == 7) {
              in_water = true;
            }
          }
        }
      }
      
      newpos = new_rel_pos + collider->get_position();
    //}
    for (int i = 0; i < 7; i ++) {
        if (old_consts[i] and mat_consts[i] == nullptr) {
          mat_consts[i] = old_mat_consts[i];
        }
        consts[i] = old_consts[i] or new_consts[i];
        old_consts[i] = new_consts[i];
    }
    
    position = newpos;
    //cout << clock() - begin << endl;
    //print(position);
}

void Entity::damage(double amount) {
  damage_health += amount;
  if (damage_health > health) {
    damage_health = health + 0.05;
  }
}

void Entity::heal(double amount, double speed) {
  healing_health += amount;
  if (healing_speed > speed or healing_speed < 0.01) {
    healing_speed = speed;
  }
  if (health + healing_health > max_health) {
    healing_health = max_health - health;
  }
}

void Entity::use_stamina(double amount) {
  if (healing_health > 0) {
    if (healing_health < amount) {
      healing_health = 0;
    } else {
      healing_health -= amount;
    }
  }
}

// void Entity::calc_constraints() {
//     //int begin = clock();
//     vec3 coords_array[3] = {{0,1,1}, {1,0,1}, {1,1,0}};
//     vec3 dir_array[3] =    {{1,0,0}, {0,1,0}, {0,0,1}};
//
//     vec3 newpos(position);
//
//     bool new_consts[7];
//     for (int i = 0; i < 7; i ++) {
//       new_consts[i] = false;
//     }
//
//     vector<Collider*> colliders;
//     find_colliders(&colliders);
//
//     vec3 newbox2 = box2 - 1.0f - axis_gap;
//     vec3 newbox1 = box1 + axis_gap;
//
//     //box2 = vec3(0,0,0);
//
//     //this system works by creating a slice of blocks that define each side of collision. the sides all share either the most positive point or the most negative point
//     //the shlice is tested to see if any blocks are int it (2) and if there are blocks, the players location is rounded to outside the block (3)
//     //axis gap make sure a side detects the collision of blocks head on before the side detectors sense the blocks. Each slice is moved out from the player by axis_gap
//     //cout << "----------------------------------" << endl;
//     //print(position);
//
//
//
//     for (Collider* collider : colliders) {
//       vec3 rel_pos = position - collider->get_position();
//       vec3 new_rel_pos = newpos - collider->get_position();
//       for (int axis = 0; axis < 3; axis ++) {
//         vec3 coords = coords_array[axis];
//         vec3 dir = dir_array[axis];
//
//         // pos is the positive point of the hitbox rounded up to the nearest block. neg is the negative most point rounded down
//         vec3 neg = floor( rel_pos + newbox1 - axis_gap*dir );// + vec3(0,1,0);
//         vec3 pos = ceil( rel_pos + newbox2 + axis_gap*dir );
//         if (axis != 1 and !consts[2]) {
//           neg = floor( rel_pos + newbox1 - axis_gap*dir + vec3(0,1,0));
//         }
//
//         //positive side
//         //pos2 is the other point for the face
//         vec3 pos2 = pos*dir + neg*coords;
//         bool constraint = false;
//         for (int x = (int)pos2.x; x < pos.x+1; x ++) {
//             for (int y = (int)pos2.y; y < pos.y+1; y ++) {
//                 for (int z = (int)pos2.z; z < pos.z+1; z ++) {
//                     Block* pix = collider->get_global(x,y,z,1);
//                     bool new_const = (pix == nullptr or pix->get() != 0);
//                     if (new_const) {
//                       //world->block_update(x,y,z);
//                     }
//                     constraint = constraint or new_const;
//                 }
//             }
//         }
//         new_consts[axis] = new_consts[axis] or constraint;
//         if (constraint) {
//             new_rel_pos[axis] = floor(rel_pos[axis] + newbox2[axis] + axis_gap) - newbox2[axis] - axis_gap;
//         }
//         vec3 neg2 = neg*dir + pos*coords;
//         if (axis == 1) {
//           //neg -= vec3(0.5,0,0.5);
//           //neg2 += vec3(0.5,0,0.5);
//           int max_y = INT_MIN;
//           bool max_y_set = false;
//           constraint = false;
//           for (int x = (int)neg.x; x < neg2.x+1; x ++) {
//               for (int y = (int)neg.y; y < neg2.y+1; y ++) {
//                   for (int z = (int)neg.z; z < neg2.z+1; z ++) {
//                       Block* pix = collider->get_global(x,y,z,1);
//                       if (pix != nullptr and pix->get() == 0) {
//                         continue;
//                       }
//                       constraint = true;
//                       for (int yoff = 1; yoff < 3; yoff ++) {
//                         pix = collider->get_global(x,y+yoff,z,1);
//                         if (pix != nullptr and pix->get() == 0) {
//                           if (y+yoff-1 > max_y) {
//                             max_y = y+yoff-1;
//                             max_y_set = true;
//                           }
//                           constraint = true;
//                           break;
//                         }
//                       }
//                   }
//               }
//           }
//
//           if (!constraint) {
//             neg = floor( rel_pos + newbox1 - axis_gap );
//             pos = ceil( rel_pos + newbox2 + axis_gap );
//             neg2 = neg*dir + pos*coords;
//
//             for (int x = (int)neg.x; x < neg2.x+1; x ++) {
//               for (int y = (int)neg.y; y < neg2.y+1; y ++) {
//                 for (int z = (int)neg.z; z < neg2.z+1; z ++) {
//                   Block* floor = collider->get_global(x,y,z,1);
//                   if (floor == nullptr or floor->get() != 0) {
//                     Block* above = collider->get_global(x,y+1,z,1);
//                     if (above != nullptr and above->get() == 0) {
//                       constraint = true;
//                     }
//                   }
//
//                 }
//               }
//             }
//
//             if (constraint) {
//               float newpos = ceil(rel_pos[axis] + newbox1[axis] - axis_gap) - newbox1[axis] + axis_gap;
//               if (std::abs(newpos - new_rel_pos[axis]) > 0.3) {
//                 constraint = false;
//               } else {
//                 new_rel_pos[axis] = newpos;
//               }
//             }
//
//           } else {
//             if (constraint and max_y_set) {
//               double newpos = max_y - newbox1[axis] + axis_gap + 1;
//               if (newpos > new_rel_pos[1]) {
//                 double diff = newpos - new_rel_pos[1];
//                 if (diff > 0.2) {
//                   diff = 0.2;
//                 }
//                 new_rel_pos[1] += diff;
//               } else {
//                 new_consts[axis+3] = false;
//               }
//               //ceil(rel_pos[axis] + newbox1[axis] - axis_gap) - newbox1[axis] + axis_gap;
//             }
//           }
//
//           new_consts[axis+3] = new_consts[axis+3] or constraint;
//
//         } else {
//           constraint = false;
//           for (int x = (int)neg.x; x < neg2.x+1; x ++) {
//               for (int y = (int)neg.y; y < neg2.y+1; y ++) {
//                   for (int z = (int)neg.z; z < neg2.z+1; z ++) {
//                       Block* pix = collider->get_global(x,y,z,1);
//                       bool new_const = (pix != NULL and pix->get() != 0);
//                       if (new_const) {
//                         //world->block_update(x,y,z);
//                       }
//                       constraint = constraint or new_const;
//                   }
//               }
//           }
//           new_consts[axis+3] = new_consts[axis+3] or constraint;
//           if (constraint) {
//             new_rel_pos[axis] = ceil(rel_pos[axis] + newbox1[axis] - axis_gap) - newbox1[axis] + axis_gap;
//           }
//         }
//       }
//       /// torso check: to tell if the constraints are only on the feet
//
//       vec3 neg = floor( rel_pos + newbox1 - axis_gap + vec3(0,1.2,0)) ;
//       vec3 pos = ceil( rel_pos + newbox2 + axis_gap );
//       bool constraint = false;
//       for (int x = (int)neg.x; x < pos.x+1; x ++) {
//         for (int y = (int)neg.y; y < pos.y+1; y ++) {
//           for (int z = (int)neg.z; z < pos.z+1; z ++) {
//             Block* pix = collider->get_global(x,y,z,1);
//             constraint = constraint or (pix != NULL and pix->get() != 0);
//           }
//         }
//       }
//       new_consts[6] =  new_consts[6] or constraint;
//
//       newpos = new_rel_pos + collider->get_position();
//     }
//     for (int i = 0; i < 7; i ++) {
//         consts[i] = old_consts[i] or new_consts[i];
//         old_consts[i] = new_consts[i];
//     }
//
//     position = newpos;
//     //cout << clock() - begin << endl;
//     //print(position);
// }

void Entity::move(vec3 change, float deltaTime) {
    bool do_drag = false;
    for (int i = 0; i < 6; i ++) {
        if (old_consts[i]) {
            vec3 dir = dirs[i];
            do_drag = true;
            vec3 component = change * dir;
            if (component.x + component.y + component.z > 0) {
                change = change - component * dir;
            }
            component = vel * dir;
            if (component.x + component.y + component.z > 0) {
                float reflection = 0;
                vec3 bounce(0,0,0);
                if (mat_consts[i] != nullptr and component.x + component.y + component.z > 5) {
                  if ((mat_consts[i]->toughness + mat_consts[i]->elastic) > 0.001) {
                    reflection = mat_consts[i]->elastic / (mat_consts[i]->toughness + mat_consts[i]->elastic);
                    if (component.x + component.y + component.z > 30) {
                      bounce = reflection * component * -0.5f;
                    }
                  }
                  //cout << reflection << ' ' << bounce.x << ' ' << bounce.y << ' ' << bounce.z << ' ' << component.x + component.y + component.z << endl;
                }
                vel = vel-component * dir + bounce * dir;
                fall_damage((component.x + component.y + component.z), (1 - reflection));
            }
        }
    }
    position += change;
    drag(do_drag, deltaTime);
}

void Entity::fall_damage(float velocity, float multiplier) {
    if (velocity > 30) {
        damage((velocity-30)/2 * multiplier);
    }
}

void Entity::drag(bool do_drag, float deltaTime) {
    if (in_water) {
      float water_drag = 1 - 8*deltaTime;
      vel *= water_drag;
    }
    if (do_drag) {
        float ground_drag = 1-10*deltaTime;
        float side_drag = 1-3*deltaTime;
        if (ground_drag < 0) {
          ground_drag = 0;
        }
        if (vel.y > 0) {
            vel *= vec3(ground_drag,1,ground_drag);
        }
        else {
            vel *= vec3(ground_drag, side_drag, ground_drag);
        }
    } else {
        float air_drag = 1-2*deltaTime;
        vel *= vec3(air_drag, 1, air_drag);
    }
}

void Entity::on_timestep(double deltatime) {
  
}

bool Entity::colliding(const Entity* other) {
  vec3 dist = other->position - position;
  vec3 box_dist = other->box1*-1.0f + box2 + 2.0f;
  vec3 box2_dist = box1*-1.0f + other->box2 + 2.0f;
  if (dist.x < 0) {
    box_dist.x = box2_dist.x;
    dist.x *= -1;
  } if (dist.y < 0) {
    box_dist.y = box2_dist.y;
    dist.y *= -1;
  } if (dist.z < 0) {
    box_dist.z = box2_dist.z;
    dist.z *= -1;
  }
  return dist.x < box_dist.x and dist.y < box_dist.y and dist.z < box_dist.z;
}

void Entity::kill() {
  
}



#endif
