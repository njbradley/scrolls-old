#ifndef ENTITY
#define ENTITY

#include "entity.h"

#include "items.h"
#include "world.h"
#include "tiles.h"
#include "blockgroups.h"
#include "materials.h"
#include "blockdata.h"
#include "cross-platform.h"
#include "game.h"
#include "blocks.h"
#include "blockiter.h"

#include <algorithm>

using namespace glm;

using namespace std;

void print(vec3 v) {
    cout << v.x << ' ' << v.y << ' ' << v.z << endl;
}





Hitbox::Hitbox(vec3 pos, vec3 nbox, vec3 pbox, quat rot, vec3 vel, quat angvel):
position(pos), negbox(nbox), posbox(pbox), rotation(rot), velocity(vel), angular_vel(angvel) {
  
}

Hitbox::Hitbox(): Hitbox(vec3(0,0,0), vec3(0,0,0), vec3(0,0,0)) {
  
}

void Hitbox::points(vec3* points) const {
  for (int i = 0; i < 8; i ++) {
    points[i] = transform_out(size() * vec3(i/4,i/2%2,i%2));
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

vec3 Hitbox::local_center() const {
  return size() / 2.0f;
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

Hitbox Hitbox::transform_in(Hitbox box) const {
  return Hitbox(
    transform_in(box.position),
    box.negbox,
    box.posbox,
    transform_in(box.rotation)
  );
}

Hitbox Hitbox::transform_out(Hitbox box) const {
  return Hitbox(
    transform_out(box.position),
    box.negbox,
    box.posbox,
    transform_out(box.rotation)
  );
}

float Hitbox::axis_projection(vec3 axis) const {
  return std::abs(glm::dot(transform_out_dir(size() * vec3(1,0,0)), axis))
       + std::abs(glm::dot(transform_out_dir(size() * vec3(0,1,0)), axis))
       + std::abs(glm::dot(transform_out_dir(size() * vec3(0,0,1)), axis));
}

bool Hitbox::collide(Hitbox other, float deltatime, float* col_time) const {
  /// using separating axis theorem:
  // https://www.jkh.me/files/tutorials/Separating%20Axis%20Theorem%20for%20Oriented%20Bounding%20Boxes.pdf
  
  vec3 axes[] = {
    dirx(), diry(), dirz(), other.dirx(), other.diry(), other.dirz(),
    glm::cross(dirx(), other.dirx()), glm::cross(dirx(), other.diry()), glm::cross(dirx(), other.dirz()),
    glm::cross(diry(), other.dirx()), glm::cross(diry(), other.diry()), glm::cross(diry(), other.dirz()),
    glm::cross(dirz(), other.dirx()), glm::cross(dirz(), other.diry()), glm::cross(dirz(), other.dirz()),
  };
  
  vec3 tvec = global_center() - other.global_center(); // vec to other box
  
  // if (glm::length(tvec) >= (glm::length(size()) + glm::length(other.size())) / 2.0f) {
  //   return false;
  // }
  if (glm::length(tvec) < (std::min(std::min(size().x, size().y), size().z)
      + std::min(std::min(other.size().x, other.size().y), other.size().z)) / 2.0f) {
    return true;
  }
  
  bool collides = true;
  float maxtime = 0;
  
  // cout << "T " << tvec << endl;
  for (vec3 axis : axes) {
    if (glm::length(axis) > 0) {
      axis = axis / glm::length(axis);
      // cout << "Axis: " << axis << endl;
      float aproj = axis_projection(axis);
      float bproj = other.axis_projection(axis);
      float tproj = std::abs(glm::dot(tvec, axis));
      // cout << aproj << ' ' << bproj << ' ' << tproj << endl;
      float overlap = tproj - (aproj + bproj) / 2.0f;
      if (overlap >= 0) {
        collides = false;
        
        float avelproj = glm::dot(axis, velocity);
        float bvelproj = glm::dot(axis, other.velocity);
        if (glm::dot(axis, tvec) > 0) {
          avelproj = -avelproj;
        } else {
          bvelproj = -bvelproj;
        }
        if (avelproj + bvelproj != 0) {
          float time = overlap / (avelproj + bvelproj);
          if (time > maxtime) {
            maxtime = time;
          }
        }
      }
    }
  }
  
  if (col_time != nullptr) {
    *col_time = maxtime;
    return collides or (maxtime >= 0 and maxtime <= deltatime);
  }
  
  return collides;
}

bool Hitbox::contains_noedge(vec3 point) const {
  point = transform_in(point);
  return 0 < point.x and point.x < size().x
     and 0 < point.y and point.y < size().y
     and 0 < point.z and point.z < size().z;
}

bool Hitbox::contains(vec3 point) const {
  point = transform_in(point);
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

ostream& operator<<(ostream& ofile, const Hitbox& hitbox) {
  ofile << "Hitbox(pos=" << hitbox.position << " negbox=" << hitbox.negbox << " posbox=" << hitbox.posbox
  << " rot=" << hitbox.rotation << ")";
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



Entity::Entity(World* nworld, vec3 pos, vec3 hitbox1, vec3 hitbox2):
world(nworld), position(pos), box1(hitbox1), box2(hitbox2), alive(true), immune(false), flying(false)
{
    angle = vec2(0,0);
    vel = vec3(0,0,0);
    lastTime = glfwGetTime();
    for (int i = 0; i < 7; i ++) {
        consts[i] = false;
        old_consts[i] = false;
    }
}

void Entity::timestep() {
        
    // Compute time difference between current and last frame
    float currentTime(glfwGetTime());
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
	lastTime = glfwGetTime();
}

void Entity::get_nearby_entities(vector<DisplayEntity*>* colliders) {
  ivec3 chunk = ivec3(position)/world->chunksize - ivec3(position.x<0, position.y<0, position.z<0);
  //std::map<ivec3,Tile*>::iterator it = world->tiles.find(chunk);
  
}



void Entity::find_colliders(vector<Collider*>* colliders) {
  ivec3 chunk = ivec3(position)/world->chunksize - ivec3(position.x<0, position.y<0, position.z<0);
  //std::map<ivec3,Tile*>::iterator it = world->tiles.find(chunk);
  vector<FallingBlockEntity*> entities;
  colliders->push_back(world);
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
    
    vector<Collider*> colliders;
    find_colliders(&colliders);
    
    vec3 newbox2 = box2 - 1.0f - axis_gap;
    vec3 newbox1 = box1 + axis_gap;
    
    //box2 = vec3(0,0,0);
    
    //this system works by creating a slice of blocks that define each side of collision. the sides all share either the most positive point or the most negative point
    //the shlice is tested to see if any blocks are int it (2) and if there are blocks, the players location is rounded to outside the block (3)
    //axis gap make sure a side detects the collision of blocks head on before the side detectors sense the blocks. Each slice is moved out from the player by axis_gap
    //cout << "----------------------------------" << endl;
    //print(position);
    
    
    
    for (Collider* collider : colliders) {
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
        //pos2 is the other point for the face
        vec3 pos2 = pos*dir + neg*coords;
        bool constraint = false;
        for (int x = (int)pos2.x; x < pos.x+1; x ++) {
            for (int y = (int)pos2.y; y < pos.y+1; y ++) {
                for (int z = (int)pos2.z; z < pos.z+1; z ++) {
                    Block* pix = collider->get_global(x,y,z,1);
                    bool new_const = is_solid_block(pix);
                    if (new_const) {
                      Material* mat = blocks->blocks[pix->pixel->value]->material;
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
                        mat = blocks->blocks[pix->pixel->value]->material;
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
                          mat = blocks->blocks[pix->pixel->value]->material;
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
                      Material* mat = blocks->blocks[floor->pixel->value]->material;
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
                        Material* mat = blocks->blocks[pix->pixel->value]->material;
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
    }
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










BlockContainer::BlockContainer(Block* b): block(b), allocated(false) {
	block->set_parent(nullptr, this, nullptr, ivec3(0,0,0), b->scale);
}

BlockContainer::BlockContainer(int scale): allocated(true) {
  block = new Block();
	block->set_parent(nullptr, this, nullptr, ivec3(0,0,0), scale);
}

BlockContainer::~BlockContainer() {
  if (allocated) {
    delete block;
  }
}

vec3 BlockContainer::get_position() const {
	return block->get_position();
}

Block* BlockContainer::get_global(int x, int y, int z, int size) {
	if (x >= 0 and y >= 0 and z >= 0 and x < block->scale and y < block->scale and z < block->scale) {
		return block->get_global(x, y, z, size);
	} else {
		return nullptr;
	}
}

void BlockContainer::set(ivec4 pos, char val, int direction, int newjoints[6]) {
  // Block* testblock;
  // while ((testblock = get_global(pos.x, pos.y, pos.z, pos.w)) == nullptr) {
  //   Pixel* pix = new Pixel(0, 0, 0, 0, block->scale * csize, nullptr, nullptr);
  //   Chunk* chunk = pix->subdivide();
  //   chunk->blocks[0][0][0] = block;
  //   block->parent = chunk;
  //   block = chunk;
  //   pix->del(true);
  //   delete pix;
  // }
  
  block->set_global(pos, pos.w, val, direction, newjoints);
}

void BlockContainer::set_global(ivec3 pos, int w, int val, int direction, int newjoints[6]) {
  block->set_global(pos, w, val, direction, newjoints);
}











DisplayEntity::DisplayEntity(World* nworld, vec3 starting_pos, vec3 hitbox1, vec3 hitbox2, Block* newblock, vec3 newblockpos, vector<DisplayEntity*> newlimbs):
Entity(nworld, starting_pos, hitbox1, hitbox2), block(newblock), render_index(-1), render_flag(true), limbs(newlimbs), blockpos(newblockpos), dead_falling(false) {
  // int size = block.block->scale;
  // box2 = vec3(size-1.2f,size-1.2f,size-1.2f);
  //block->render(&vecs, this, 0, 0, 0);
}

DisplayEntity::DisplayEntity(World* nworld, vec3 starting_pos, vec3 hitbox1, vec3 hitbox2, Block* newblock, vec3 newblockpos):
Entity(nworld, starting_pos, hitbox1, hitbox2), block(newblock), render_index(-1), render_flag(true), blockpos(newblockpos), dead_falling(false) {
  // int size = block.block->scale;
  // box2 = vec3(size-1.2f,size-1.2f,size-1.2f);
  //block->render(&vecs, this, 0, 0, 0);
}

DisplayEntity::DisplayEntity(World* nworld, istream& ifile):
Entity(nworld, vec3(0,0,0), vec3(0,0,0), vec3(0,0,0)), block(nullptr), render_index(-1), render_flag(true), blockpos(0,0,0), dead_falling(false) {
  ifile >> position.x >> position.y >> position.z;
  ifile >> vel.x >> vel.y >> vel.z;
  ifile >> angle.x >> angle.y;
  ifile >> health >> max_health >> damage_health >> healing_health >> healing_speed;
  ifile >> emitted_light;
  ifile >> flying >> alive >> immune >> dead_falling;
}


void DisplayEntity::to_file(ostream& ofile) {
  ofile << position.x << ' ' << position.y << ' ' << position.z << endl;
  ofile << vel.x << ' ' << vel.y << ' ' << vel.z << endl;
  ofile << angle.x << ' ' << angle.y << endl;
  ofile << health << ' ' << max_health << ' ' << damage_health << ' ' << healing_health << ' ' << healing_speed << endl;
  ofile << emitted_light << endl;
  ofile << flying << ' ' << alive << ' ' << immune << ' ' << dead_falling << endl;
}

void DisplayEntity::render(RenderVecs* allvecs) {
  // vecs.clear();
  // block->all([=] (Pixel* pix) {
  //   pix->set_render_flag();
  //   pix->render_index = pair<int,int>(1,0);
  // });
  // block->render(&vecs, int(position.x), int(position.y), int(position.z));
  //
  /*
  
  if (render_flag) {
    vecs.clear();
    for (Pixel* pix : block.block->iter()) {
      pix->render_index = RenderIndex::npos;
      pix->parbl->set_render_flag();
      pix->sunlight = lightmax;
    }
    block.block->render(&vecs, &vecs, 0xff, true);
    for (int i = 0; i < vecs.num_verts; i++) {
      vecs.verts[i*3+0] += blockpos.x;
      vecs.verts[i*3+1] += blockpos.y;
      vecs.verts[i*3+2] += blockpos.z;
    }
    // for (DisplayEntity* limb : limbs) {
    //   cout << "lib:";
    //   print(limb->position);
    //   limb->render_flag = true;
    //   limb->render_index = RenderIndex::npos;
    //   limb->render(&vecs);
    //   cout << vecs.num_verts << '-' << endl;
    // }
    render_flag = false;
  }
  
  if (vecs.num_verts > 0) {
    MemVecs translated;
    translated.add(&vecs);
    //cout << 't' << translated.num_verts << endl;
    
    
    for (DisplayEntity* limb : limbs) {
      limb->render_index = RenderIndex::npos;
      limb->render(&translated);
    }
    //cout << 'y' << translated.num_verts << endl;
    for (int i = 0; i < translated.num_verts; i++) {
      // translated.verts[i*3+0] += blockpos.x;
      // translated.verts[i*3+1] += blockpos.y;
      // translated.verts[i*3+2] += blockpos.z;
      float x = translated.verts[i*3+0];
      float y = translated.verts[i*3+1];
      float z = translated.verts[i*3+2];
      translated.verts[i*3+0] = x * cos(angle.y) - y * sin(angle.y);
      translated.verts[i*3+1] = x * sin(angle.y) + y * cos(angle.y);
      x = translated.verts[i*3+0];
      y = translated.verts[i*3+1];
      z = translated.verts[i*3+2];
      translated.verts[i*3+0] = x * cos(angle.x) - z * sin(angle.x);
      translated.verts[i*3+2] = x * sin(angle.x) + z * cos(angle.x);
      
      translated.verts[i*3+0] += position.x;
      translated.verts[i*3+1] += position.y;
      translated.verts[i*3+2] += position.z;
      
      translated.light[i*2+0] = int(translated.light[i*2+0]*sunlight)/lightmax;
      if (translated.light[i*2+1] == 0) {
        translated.light[i*2+1] = blocklight/lightmax;
      }
    }
    if (render_index.isnull()) {
      render_index = allvecs->add(&translated);
    } else {
      allvecs->edit(render_index, &translated); //ERR: passes problemic vector? segfault deep in allvecs.edit, where translated.verts is added on
    }
  }*/
  //cout << render_index.first << ',' << render_index.second << ' ' << vecs.num_verts << endl;
  // cout << position.x << ' ' << position.y << ' ' << position.z << endl;
  // for (bool b : consts) {
  //   cout << b << ' ';
  // }
  // cout << endl;
}


void DisplayEntity::calc_light(vec3 offset, vec2 ang) {
  vec3 rotpos;
  float tmpx;
  tmpx = position.x * cos(ang.y) - position.y * sin(ang.y);
  rotpos.y = position.x * sin(ang.y) + position.y * cos(ang.y);
  
  rotpos.x = tmpx * cos(ang.x) - position.z * sin(ang.x);
  rotpos.z = tmpx * sin(ang.x) + position.z * cos(ang.x);
  
  Block* b = world->get_global(rotpos.x+offset.x, rotpos.y+offset.y, rotpos.z+offset.z, 1);
    
  
  if (emitted_light == 0 and !emitted_light_flag) {
    if (b != nullptr) {
      sunlight = b->pixel->sunlight;
      blocklight = b->pixel->blocklight;
    } else {
      sunlight = 10;
      blocklight = 0;
    }
  } else {
    if (b != nullptr) {
      Pixel* pix = b->pixel;
      ivec3 newpos = pix->parbl->globalpos;
      sunlight = pix->sunlight;
      if (newpos != lit_block or emitted_light_flag) {
        Block* bb = world->get_global(lit_block.x, lit_block.y, lit_block.z, 1);
        if (bb != nullptr) {
          bb->pixel->entitylight = 0;
          bb->pixel->render_update();
        }
        if (pix->blocklight < emitted_light) {
          pix->entitylight = emitted_light;
          pix->render_update();
          lit_block = newpos;
        }
      }
      if (pix->blocklight < emitted_light) {
        blocklight = emitted_light;
      } else {
        blocklight = pix->blocklight;
      }
    } else {
      sunlight = 10;
      blocklight = emitted_light;
    }
  }
  emitted_light_flag = false;
  
  for (DisplayEntity* limb : limbs) {
    limb->calc_light(offset+rotpos, ang+angle);
  }
}

void DisplayEntity::on_timestep(double deltatime) {
  calc_light(vec3(0,0,0), vec2(0,0));
  render(&world->glvecs);
  //vel += vec3(0.0,0,-0.08);
}

DisplayEntity::~DisplayEntity() {
  //cout << render_index.first << ',' << render_index.second << " deleted! " << endl;
  // position.y -= 1000;
  // render();
  if (emitted_light > 0) {
    Block* b = world->get_global(lit_block.x, lit_block.y, lit_block.z, 1);
    if (b != nullptr) {
      b->pixel->entitylight = 0;
      b->pixel->render_update();
    }
  }
  if (!render_index.isnull()) {
    world->glvecs.del(render_index);
    //world->dead_render_indexes.push_back(render_index);
  }
  if (block.block != nullptr) {
    delete block.block;
  }
  
  for (DisplayEntity* limb : limbs) {
    limb->render_index = RenderIndex::npos;
    delete limb;
  }
}

#endif
