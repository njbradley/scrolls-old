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




Entity::Entity(vec3 pos, vec3 hitbox1, vec3 hitbox2):
position(pos), box1(hitbox1), box2(hitbox2-vec3(1,1,1)), alive(true)
{
    angle = vec2(0,0);
    vel = vec3(0,0,0);
    lastTime = glfwGetTime();
    for (int i = 0; i < 7; i ++) {
        consts[i] = false;
        old_consts[i] = false;
    }
}

void Entity::timestep(World* world) {
        
    on_timestep(world);
    // Compute time difference between current and last frame
    float currentTime(glfwGetTime());
    deltaTime = (currentTime - lastTime);
    if (deltaTime > 0.2f) {
        deltaTime = 0.2f;
        cout << "warning: had to drop ticks" << endl;
    }
    //cout << vel.x << ' ' << vel.y << ' ' << vel.z << endl;
    if (!flying) {
        calc_constraints(world);
        vel += vec3(0,-18,0) * deltaTime;
    } else {
        vel.y = 0;
    }
    //for (bool b : consts) {
    //    cout << b << ' ';
    //}
    //cout << endl;
    //cout << deltaTime << endl;
    move(vel*deltaTime, deltaTime);
    lastTime = currentTime;
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
  colliders->push_back(world);
  ivec3 chunk = ivec3(position)/world->chunksize - ivec3(position.x<0, position.y<0, position.z<0);
  //std::map<ivec3,Tile*>::iterator it = world->tiles.find(chunk);
  vector<DisplayEntity*> entities;
  get_nearby_entities(&entities);
  // for (pair<ivec3, Tile*> kvpair : world->tiles) {
  //   if (glm::length(vec3(kvpair.first-chunk)) <= 1.5) {
  //     //print (kvpair.first);
      for (DisplayEntity* e : entities) {
        if (e != this and colliding(e)) {
          colliders->push_back(e);
        }
      }
    //}
  //}
}

void Entity::calc_constraints(World* world) {
    
    vec3 coords_array[3] = {{0,1,1}, {1,0,1}, {1,1,0}};
    vec3 dir_array[3] =    {{1,0,0}, {0,1,0}, {0,0,1}};
    
    vec3 newpos(position);
    
    bool new_consts[7];
    for (int i = 0; i < 7; i ++) {
      new_consts[i] = false;
    }
    
    vector<Collider*> colliders;
    find_colliders(&colliders);
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
        vec3 neg = floor( rel_pos + box1 - axis_gap*dir );// + vec3(0,1,0);
        vec3 pos = ceil( rel_pos + box2 + axis_gap*dir );
        
        
        //positive side
        //pos2 is the other point for the face
        vec3 pos2 = pos*dir + neg*coords;
        bool constraint = false;
        for (int x = (int)pos2.x; x < pos.x+1; x ++) {
            for (int y = (int)pos2.y; y < pos.y+1; y ++) {
                for (int z = (int)pos2.z; z < pos.z+1; z ++) {
                    Block* pix = collider->get_global(x,y,z,1);
                    constraint = constraint or (pix != NULL and pix->get() != 0);
                }
            }
        }
        new_consts[axis] = new_consts[axis] or constraint;
        if (constraint) {
            new_rel_pos[axis] = floor(new_rel_pos[axis] + box2[axis] + axis_gap) - box2[axis] - axis_gap;
        }
        vec3 neg2 = neg*dir + pos*coords;
        constraint = false;
        for (int x = (int)neg.x; x < neg2.x+1; x ++) {
            for (int y = (int)neg.y; y < neg2.y+1; y ++) {
                for (int z = (int)neg.z; z < neg2.z+1; z ++) {
                    Block* pix = collider->get_global(x,y,z,1);
                    bool new_const = (pix != NULL and pix->get() != 0);
                    constraint = constraint or new_const;
                }
            }
        }
        new_consts[axis+3] = new_consts[axis+3] or constraint;
        if (constraint) {
          new_rel_pos[axis] = ceil(new_rel_pos[axis] + box1[axis] - axis_gap) - box1[axis] + axis_gap;
        }
      }
      /// torso check: to tell if the constraints are only on the feet
      
      vec3 neg = floor( rel_pos + box1 - axis_gap + vec3(0,1.2,0)) ;
      vec3 pos = ceil( rel_pos + box2 + axis_gap );
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
    //print(position);
}

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
    if (velocity > 20) {
        health -= (int)(velocity-20)/2;
    }
}

void Entity::drag(bool do_drag, float deltaTime) {
    if (do_drag) {
        float ground_drag = 1-5*deltaTime;
        if (vel.y > 0) {
            vel *= vec3(ground_drag,1,ground_drag);
        }
        else {
            vel *= ground_drag;
        }
    } else {
        float air_drag = 1-3*deltaTime;
        vel *= vec3(air_drag, 1, air_drag);
    }
}

void Entity::on_timestep(World* world) {
  
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




DisplayEntity::DisplayEntity(vec3 starting_pos, Block* newblock): Entity(starting_pos, vec3(0.2,0.2,0.2), vec3(1,1,1)), block(newblock), render_index(-1,0) {
  int size = block->scale;
  box2 = vec3(size-1.2f,size-1.2f,size-1.2f);
  block->render(&vecs, this, 0, 0, 0);
}

vec3 DisplayEntity::get_position() {
  return position;
}

Block * DisplayEntity::get_global(int x, int y, int z, int size) {
  ivec3 index(x,y,z);
  //print (index);
  index -= ivec3(box1-axis_gap);
  //print (position);
  //print (index);
  int scale = block->scale;
  //cout << ';' << endl;
  if ( index.x >= 0 and index.x < scale and index.y >= 0 and index.y < scale and index.z >= 0 and index.z < scale ) {
    return block->get_global(index.x, index.y, index.z,size);
  } else {
    return nullptr;
  }
}

void DisplayEntity::render() {
  // vecs.clear();
  // block->all([=] (Pixel* pix) {
  //   pix->set_render_flag();
  //   pix->render_index = pair<int,int>(1,0);
  // });
  // block->render(&vecs, int(position.x), int(position.y), int(position.z));
  //
  MemVecs translated;
  translated.add(&vecs);
  for (int i = 0; i < translated.num_verts; i++) {
    translated.verts[i*3+0] += position.x;
    translated.verts[i*3+1] += position.y;
    translated.verts[i*3+2] += position.z;
  }
  
  if (render_index == pair<int,int>(-1,0)) {
    render_index = world->glvecs.add(&translated);
  } else {
    world->glvecs.edit(render_index, &translated);
  }
  
  
  // cout << position.x << ' ' << position.y << ' ' << position.z << endl;
  // for (bool b : consts) {
  //   cout << b << ' ';
  // }
  // cout << endl;
}

void DisplayEntity::on_timestep(World* world) {
  render();
  //vel += vec3(0.0,0,-0.08);
}

void DisplayEntity::calc_constraints(World* world) {
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
    
    block->all([&] (Pixel* pix) {
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
          side = block->get_global(gx, gy, gz, pix->scale);
          bool out_bounds = gx >= block->scale or gy >= block->scale or gz >= block->scale;
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
          side = block->get_global(gx, gy, gz, pix->scale);
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
    
    for (int i = 0; i < 7; i ++) {
        consts[i] = old_consts[i] or new_consts[i];
        old_consts[i] = new_consts[i];
    }
    
    position = newpos;
}

void DisplayEntity::die() {
  world->glvecs.del(render_index);
}





NamedEntity::NamedEntity(vec3 starting_pos, string name): DisplayEntity(starting_pos, loadblock(name)), nametype(name)  {
  
}

Block* NamedEntity::loadblock(string name) {
  std::ifstream ifile(entitystorage->blocks[name]);
  if (!ifile.good()) {
    cout << "file error code s9d8f0s98d0f9s80f98sd0" << endl;
  }
  int size;
  ifile >> size;
  box2 = vec3(size-1.2f,size-1.2f,size-1.2f);
  char buff;
  ifile.read(&buff,1);
  return Block::from_file(ifile, 0, 0, 0, size, nullptr, nullptr);
}



FallingBlockEntity::FallingBlockEntity(BlockGroup* newgroup): DisplayEntity(newgroup->position, newgroup->block), group(newgroup) {
  
}

void FallingBlockEntity::on_timestep(World* world) {
  render();
  if (consts[4]) {
    group->copy_to_world(position);
    delete group;
    alive = false;
  }
}
    




EntityStorage::EntityStorage() {
  vector<string> paths;
  get_files_folder("resources/data/entities", &paths);
  for (string path : paths) {
    ifstream ifile(path);
    std::stringstream ss(path);
    string name;
    std::getline(ss, name, '.');
    blocks[name] = "resources/data/entities/" + path;
    cout << "loaded " << name << " from file" << endl;
  }
}

#endif
