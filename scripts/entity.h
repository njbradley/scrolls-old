#ifndef ENTITY
#define ENTITY

#include <iostream>
#include "blocks-predef.h"
#include "items.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

#include "entity-predef.h"
#include "tiles-predef.h"
#include "blockphysics-predef.h"

using namespace glm;

using namespace std;

void print(vec3 v) {
    cout << v.x << ' ' << v.y << ' ' << v.z << endl;
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
    //cout << vel.x << ' ' << vel.y << ' ' << vel.z << endl;
    move(vel*deltaTime, deltaTime);
    if (!flying) {
        calc_constraints();
        vel += vec3(0,-40,0) * deltaTime;
    } else {
        vel.y = 0;
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
  for (pair<ivec3, Tile*> kvpair : world->tiles) {
    if (glm::length(vec3(kvpair.first-chunk)) <= 1.5) {
      //print (kvpair.first);
      for (DisplayEntity* e : kvpair.second->entities) {
        colliders->push_back(e);
      }
    }
  }
}



void Entity::find_colliders(vector<Collider*>* colliders) {
  ivec3 chunk = ivec3(position)/world->chunksize - ivec3(position.x<0, position.y<0, position.z<0);
  //std::map<ivec3,Tile*>::iterator it = world->tiles.find(chunk);
  vector<FallingBlockEntity*> entities;
  for (pair<ivec3, Tile*> kvpair : world->tiles) {
    if (glm::length(vec3(kvpair.first-chunk)) <= 1.5) {
      //print (kvpair.first);
      for (FallingBlockEntity* e : kvpair.second->block_entities) {
        if (e != this and colliding(e)) {
          colliders->push_back(e);
        }
      }
    }
  }
  
    colliders->push_back(world);
}

void Entity::tick() {
  
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
                    bool new_const = (pix != NULL and pix->get() != 0);
                    if (new_const) {
                      //world->block_update(x,y,z);
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
                      if (pix == nullptr or pix->get() == 0) {
                        continue;
                      }
                      constraint = true;
                      for (int yoff = 1; yoff < 3; yoff ++) {
                        pix = collider->get_global(x,y+yoff,z,1);
                        if (pix == nullptr or pix->get() == 0) {
                          if (y+yoff-1 > max_y) {
                            max_y = y+yoff-1;
                            max_y_set = true;
                          }
                          constraint = true;
                          break;
                        }
                      }
                  }
              }
          }
          
          if (!constraint) {
            neg = floor( rel_pos + newbox1 - axis_gap );
            pos = ceil( rel_pos + newbox2 + axis_gap );
            neg2 = neg*dir + pos*coords;
            
            for (int x = (int)neg.x; x < neg2.x+1; x ++) {
              for (int y = (int)neg.y; y < neg2.y+1; y ++) {
                for (int z = (int)neg.z; z < neg2.z+1; z ++) {
                  Block* floor = collider->get_global(x,y,z,1);
                  if (floor != nullptr and floor->get() != 0) {
                    Block* above = collider->get_global(x,y+1,z,1);
                    if (above == nullptr or above->get() == 0) {
                      constraint = true;
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
                      bool new_const = (pix != NULL and pix->get() != 0);
                      if (new_const) {
                        //world->block_update(x,y,z);
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
      
      vec3 neg = floor( rel_pos + newbox1 - axis_gap + vec3(0,1.2,0)) ;
      vec3 pos = ceil( rel_pos + newbox2 + axis_gap );
      bool constraint = false;
      for (int x = (int)neg.x; x < pos.x+1; x ++) {
        for (int y = (int)neg.y; y < pos.y+1; y ++) {
          for (int z = (int)neg.z; z < pos.z+1; z ++) {
            Block* pix = collider->get_global(x,y,z,1);
            constraint = constraint or (pix != NULL and pix->get() != 0);
          }
        }
      }
      new_consts[6] =  new_consts[6] or constraint;
      
      newpos = new_rel_pos + collider->get_position();
    }
    for (int i = 0; i < 7; i ++) {
        consts[i] = old_consts[i] or new_consts[i];
        old_consts[i] = new_consts[i];
    }
    
    position = newpos;
    //cout << clock() - begin << endl;
    //print(position);
}

void Entity::damage(int amount) {
  health -= amount;
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
                vel = vel-component*dir;
                fall_damage(component.x + component.y + component.z);
            }
        }
    }
    position += change;
    drag(do_drag, deltaTime);
}

void Entity::fall_damage(float velocity) {
    if (velocity > 30) {
        health -= (int)(velocity-30)/2;
    }
}

void Entity::drag(bool do_drag, float deltaTime) {
    if (do_drag) {
        float ground_drag = 1-20*deltaTime;
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
        float air_drag = 1;//-5*deltaTime;
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




DisplayEntity::DisplayEntity(World* nworld, vec3 starting_pos, vec3 hitbox1, vec3 hitbox2, Block* newblock, vec3 newblockpos, vector<DisplayEntity*> newlimbs):
Entity(nworld, starting_pos, hitbox1, hitbox2), block(newblock), render_index(-1,0), render_flag(true), limbs(newlimbs), blockpos(newblockpos), dead_falling(false) {
  // int size = block.block->scale;
  // box2 = vec3(size-1.2f,size-1.2f,size-1.2f);
  //block->render(&vecs, this, 0, 0, 0);
}

DisplayEntity::DisplayEntity(World* nworld, vec3 starting_pos, vec3 hitbox1, vec3 hitbox2, Block* newblock, vec3 newblockpos):
Entity(nworld, starting_pos, hitbox1, hitbox2), block(newblock), render_index(-1,0), render_flag(true), blockpos(newblockpos), dead_falling(false) {
  // int size = block.block->scale;
  // box2 = vec3(size-1.2f,size-1.2f,size-1.2f);
  //block->render(&vecs, this, 0, 0, 0);
}

DisplayEntity::DisplayEntity(World* nworld, istream& ifile):
Entity(nworld, vec3(0,0,0), vec3(0,0,0), vec3(0,0,0)), block(nullptr), render_index(-1,0), render_flag(true), blockpos(0,0,0), dead_falling(false) {
  ifile >> position.x >> position.y >> position.z;
  ifile >> vel.x >> vel.y >> vel.z;
  ifile >> angle.x >> angle.y;
  ifile >> health;
  ifile >> flying >> alive >> immune >> dead_falling;
}


void DisplayEntity::to_file(ostream& ofile) {
  ofile << position.x << ' ' << position.y << ' ' << position.z << endl;
  ofile << vel.x << ' ' << vel.y << ' ' << vel.z << endl;
  ofile << angle.x << ' ' << angle.y << endl;
  ofile << health << endl;
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
  
  
  
  if (render_flag) {
    vecs.clear();
    block.block->all([=] (Pixel* pix) {
      pix->render_index = pair<int,int>(-1,0);
      pix->set_render_flag();
    });
    block.block->render(&vecs, &block, 0, 0, 0);
    for (int i = 0; i < vecs.num_verts; i++) {
      vecs.verts[i*3+0] += blockpos.x;
      vecs.verts[i*3+1] += blockpos.y;
      vecs.verts[i*3+2] += blockpos.z;
    }
    // for (DisplayEntity* limb : limbs) {
    //   cout << "lib:";
    //   print(limb->position);
    //   limb->render_flag = true;
    //   limb->render_index = pair<int,int>(-1,0);
    //   limb->render(&vecs);
    //   cout << vecs.num_verts << '-' << endl;
    // }
    render_flag = false;
  }
  MemVecs translated;
  translated.add(&vecs);
  //cout << 't' << translated.num_verts << endl;
  
  
  for (DisplayEntity* limb : limbs) {
    limb->render_index = pair<int,int>(-1,0);
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
    
    translated.light[i*2+0] = int(translated.light[i*2+0]*sunlight)/10.0f;
    if (translated.light[i*2+1] == 0) {
      translated.light[i*2+1] = blocklight/10.0f;
    }
  }
  
  if (render_index == pair<int,int>(-1,0)) {
    render_index = allvecs->add(&translated);
  } else {
    allvecs->edit(render_index, &translated); //ERR: passes problemic vector? segfault deep in allvecs.edit, where translated.verts is added on
  }
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
  if (b != nullptr) {
    sunlight = b->get_pix()->sunlight;
    blocklight = b->get_pix()->blocklight;
  } else {
    sunlight = 10;
    blocklight = 0;
  }
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
  if (render_index.first != -1) {
    //world->glvecs.del(render_index);
    world->dead_render_indexes.push_back(render_index);
  }
  if (block.block != nullptr) {
    block.block->del(false);
    delete block.block;
  }
  
  for (DisplayEntity* limb : limbs) {
    limb->render_index = pair<int,int>(-1,0);
    delete limb;
  }
}





NamedEntity::NamedEntity(World* nworld, vec3 starting_pos, vec3 hitbox1, vec3 hitbox2, string name, vec3 newblockpos,
  vector<DisplayEntity*> newlimbs):
  DisplayEntity(nworld, starting_pos, hitbox1, hitbox2, loadblock(name), newblockpos, newlimbs), nametype(name), pointing(0) {
  health = 1;
  //block->rotate(1, 1);
}

NamedEntity::NamedEntity(World* nworld, vec3 starting_pos, vec3 hitbox1, vec3 hitbox2, string name, vec3 newblockpos):
  DisplayEntity(nworld, starting_pos, hitbox1, hitbox2, loadblock(name), newblockpos), nametype(name), pointing(0) {
  health = 1;
  //block->rotate(1, 1);
}

NamedEntity::NamedEntity(World* nworld, istream& ifile): DisplayEntity(nworld, ifile) {
  
}

Block* NamedEntity::loadblock(string name) {
  std::ifstream ifile(entitystorage->blocks[name], ios::binary);
  if (!ifile.good()) {
    cout << "file error code with " << name << " -- s9d8f0s98d0f9s80f98sd0" << endl;
    hard_crash(29348023948029834);
  }
  int size;
  ifile >> size;
  box2 = vec3(size-1.2f,size-1.2f,size-1.2f);
  char buff;
  ifile.read(&buff,1);
  return Block::from_file(ifile, 0, 0, 0, size, nullptr, nullptr);
}

void NamedEntity::on_timestep(double deltatime) {
  DisplayEntity::on_timestep(deltatime);
  return;
  vec3 dist_to_player = world->player->position - (position + box2 / 2.0f);
  double lon =  atan2(dist_to_player.z, dist_to_player.x);
  //double lat = atan2(dist_to_player.y, vec2(dist_to_player.x, dist_to_player.z).length());
  //cout << angle << ' ' << pointing << endl;
  // if (angle < 0) {
  //   angle += 4;
  // }
  angle.x += float(rand()%10-5)/100;
  return;
  //angle.y = lat;
  
  float dist = glm::length(dist_to_player);
  dist_to_player /= dist;
  if (consts[4]) {
    vel.x += 0.5 * dist_to_player.x;
    vel.z += 0.5 * dist_to_player.z;
  } else {
    vel.x += 0.1 * dist_to_player.x;
    vel.z += 0.1 * dist_to_player.z;
  }
  if (consts[4] and (consts[0] or consts[2] or consts[3] or consts[5])) {
    vel.y = 20;
  }
  if (colliding(world->player)) {
    world->player->health -= 1;
    world->player->vel += (dist_to_player + vec3(0,1,0)) * 5.0f;
    vel -= (dist_to_player + vec3(0,1,0)) * 3.0f;
  }
  if (health < 0) {
    for (int x = 0; x < block.block->scale; x ++) {
      for (int y = 0; y < block.block->scale; y ++) {
        for (int z = 0; z < block.block->scale; z ++) {
          Pixel* pix = block.block->get_global(x, y, z, 1)->get_pix();
          if (pix->value != 0) {
            world->set(position.x+x, position.y+y, position.z+z, pix->value, pix->direction);
          }
        }
      }
    }
    alive = false;
  }
}



FallingBlockEntity::FallingBlockEntity(World* nworld, BlockGroup* newgroup): DisplayEntity(nworld, newgroup->position, vec3(0,0,0),
vec3(1,1,1), newgroup->block, vec3(0,0,0)), group(newgroup) {
  box2 = vec3(block.block->scale - 0.2, block.block->scale, block.block->scale);
  immune = true;
}

FallingBlockEntity::FallingBlockEntity(World* nworld, istream& ifile): DisplayEntity(nworld, ifile) {
  group = BlockGroup::from_file(nworld, ifile);
  block = group->block;
  // string buff;
  // int scale;
  // ifile >> scale;
  // getline(ifile, buff, ':');
  // group->block = Block::from_file(ifile, 0, 0, 0, scale, nullptr, nullptr);
}

void FallingBlockEntity::to_file(ostream& ofile) {
  DisplayEntity::to_file(ofile);
  group->to_file(ofile);
}

void FallingBlockEntity::on_timestep(double deltatime) {
  DisplayEntity::on_timestep(deltatime);
  if (consts[4]) {
    group->copy_to_world(position);
    delete group;
    block.block = nullptr;
    alive = false;
  }
}
    
Block * FallingBlockEntity::get_global(int x, int y, int z, int size) {
  ivec3 index(x,y,z);
  //print (index);
  index -= ivec3(box1-axis_gap);
  //print (position);
  //print (index);
  int scale = block.block->scale;
  //cout << ';' << endl;
  if ( index.x >= 0 and index.x < scale and index.y >= 0 and index.y < scale and index.z >= 0 and index.z < scale ) {
    return block.block->get_global(index.x, index.y, index.z,size);
  } else {
    return nullptr;
  }
}

vec3 FallingBlockEntity::get_position() const {
  return position;
}

void FallingBlockEntity::calc_constraints() {
    float axis_gap = 0.2;
    
    vec3 coords_array[3] = {{0,1,1}, {1,0,1}, {1,1,0}};
    vec3 dir_array[3] =    {{1,0,0}, {0,1,0}, {0,0,1}};
    
    vec3 newpos(position);
    
    bool new_consts[7] = {0,0,0,0,0,0,0};
    
    //box2 = vec3(0,0,0);
    
    //this system works by creating a slice of blocks that define each side of collision. the sides all share either the most positive point or the most negative point
    //the shlice is tested to see if any blocks are int it (2) and if there are blocks, the players location is rounded to outside the block (3)
    //axis gap make sure a side detects the collision of blocks head on before the side detectors sense the blocks. Each slice is moved out from the player by axis_gap
    //cout << "----------------------------------" << endl;
    //print(position);
    if (block.block == nullptr) {
      return;
    }
    block.block->all([&] (Pixel* pix) {
      if (!pix->is_air(0,0,0)) {
        int bx, by, bz;
        pix->global_position(&bx, &by, &bz);
        vec3 block_box1(bx+axis_gap, by+axis_gap, bz+axis_gap);
        vec3 block_box2(bx+pix->scale-axis_gap-1, by+pix->scale-axis_gap-1, bz+pix->scale-axis_gap-1);
        for (int axis = 0; axis < 3; axis ++) {
          vec3 coords = coords_array[axis];
          vec3 dir = dir_array[axis];
          
          // pos is the positive point of the hitbox rounded up to the nearest block. neg is the negative most point rounded down
          vec3 neg = floor( position + block_box1 - axis_gap*dir );// + vec3(0,1,0);
          vec3 pos = ceil( position + block_box2 + axis_gap*dir );
          //cout << '-' << endl;
          // print(neg);
          // print(pos);
          Block* side;
          int gx, gy, gz;
          
          //positive side
          //pos2 is the other point for the face
          gx = bx+int(dir.x)*pix->scale;
          gy = by+int(dir.y)*pix->scale;
          gz = bz+int(dir.z)*pix->scale;
          //cout << gx << ' ' << gy << ' ' << gz << endl;
          side = block.block->get_global(gx, gy, gz, pix->scale);
          bool out_bounds = gx >= block.block->scale or gy >= block.block->scale or gz >= block.block->scale;
          //cout << out_bounds << endl;
          if (side == nullptr or out_bounds or side->is_air(-dir.x, -dir.y, -dir.z)) {
            vec3 pos2 = pos*dir + neg*coords;
            bool constraint = false;
            //cout << "pos2 ";
            //print(pos2);
            //cout << pos2.x << ' ' << pos.x+1 << ' ' << pos2.y << ' ' << pos.y+1 << ' ' << pos2.z << ' ' << pos.z+1 << endl;
            for (int x = (int)pos2.x; x < pos.x+1; x ++) {
              for (int y = (int)pos2.y; y < pos.y+1; y ++) {
                for (int z = (int)pos2.z; z < pos.z+1; z ++) {
                  //crash(1);
                  Block* pix = world->get_global(x,y,z,1);
                  constraint = constraint or (pix != NULL and pix->get() != 0);
                }
              }
            }
            new_consts[axis] = new_consts[axis] or constraint;
            if (constraint) {
              //cout << "move pos" << endl;
              newpos[axis] = floor(position[axis] + block_box2[axis] + axis_gap) - block_box2[axis] - axis_gap;
            }
          }
          
          gx = bx-int(dir.x)*pix->scale;
          gy = by-int(dir.y)*pix->scale;
          gz = bz-int(dir.z)*pix->scale;
          //cout << gx << ' ' << gy << ' ' << gz << endl;
          side = block.block->get_global(gx, gy, gz, pix->scale);
          out_bounds = gx < 0 or gy < 0 or gz < 0;
          if (side == nullptr or out_bounds or side->is_air(dir.x, dir.y, dir.z)) {
            vec3 neg2 = neg*dir + pos*coords;
            //cout << "neg2 ";
            //print(neg2);
            bool constraint = false;
            for (int x = (int)neg.x; x < neg2.x+1; x ++) {
              for (int y = (int)neg.y; y < neg2.y+1; y ++) {
                for (int z = (int)neg.z; z < neg2.z+1; z ++) {
                  Block* pix = world->get_global(x,y,z,1);
                  constraint = constraint or (pix != NULL and pix->get() != 0);
                }
              }
            }
            new_consts[axis+3] = new_consts[axis+3] or constraint;
            if (constraint) {
              //cout << "move neg" << endl;
              newpos[axis] = ceil(position[axis] + block_box1[axis] - axis_gap) - block_box1[axis] + axis_gap;
            }
          }
        }
      }
    });
    
    
    
    for (DisplayEntity* limb : limbs) {
      vec3 limbpos = limb->position;
      limb->position += position;
      limb->calc_constraints();
      vec3 diff = limb->position - position - limbpos;
      //print (diff);
      newpos += diff;
      limb->position = limbpos;
      for (int i = 0; i < 7; i ++) {
        new_consts[i] = new_consts[i] or limb->consts[i];
      }
    }
    
    for (int i = 0; i < 7; i ++) {
        consts[i] = old_consts[i] or new_consts[i];
        old_consts[i] = new_consts[i];
    }
    
    position = newpos;
}



EntityStorage::EntityStorage() {
  vector<string> paths;
  get_files_folder("resources/data/entities/blockfiles", &paths);
  for (string path : paths) {
    ifstream ifile(path);
    std::stringstream ss(path);
    string name;
    std::getline(ss, name, '.');
    blocks[name] = "resources/data/entities/blockfiles/" + path;
    //cout << "loaded " << name << " from file" << endl;
  }
  cout << "loaded " << paths.size() << " entities from file" << endl;
}

#endif
