#ifndef ENTITY
#define ENTITY

#include "entity.h"

#include "world.h"
#include "blockdata.h"
#include "blocks.h"
#include "blockiter.h"
#include "debug.h"
#include "graphics.h"
#include "fileformat.h"

DEFINE_PLUGIN(Entity);

Entity::Entity(): FreeBlock() {
  
}

Entity::Entity(istream& ifile) {
  FreeBlock::from_file(ifile);
}

Entity::~Entity() {
  
}


void Entity::set_position(vec3 pos, quat rot) {
  Movingbox newbox = box;
  newbox.position = pos;
  newbox.rotation = rot;
  set_box(newbox);
}

void Entity::to_file(ostream& ofile) const {
  ofile << blockformat::entity;
  FileFormat::write_fixed(ofile, get_plugin_id().id);
  FreeBlock::to_file(ofile);
}

void Entity::timestep(float curtime, float deltatime) {
  FreeBlock::timestep(curtime, deltatime);
}

Entity* Entity::entity_cast() {
  return this;
}

Entity* Entity::create_from_id(istream& ifile) {
  if (ifile.peek() == blockformat::entity) {
    ifile.get();
  }
  PluginId id;
  FileFormat::read_fixed(ifile, &id.id);
  // cout << id.name() << endl;
  // cout << (int) ifile.peek() << ' ' << &ifile << endl;
  Entity* result = Entity::plugnew(id, ifile);
  // cout << result << endl;
  return result;
}


/*
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
*/


#endif
