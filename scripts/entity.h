#ifndef ENTITY
#define ENTITY

#include <iostream>
#include "blocks-predef.h"
#include "items.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

#include "entity-predef.h"

using namespace glm;

using namespace std;

void print(vec3 v) {
    cout << v.x << ' ' << v.y << ' ' << v.z << endl;
}


Entity::Entity(vec3 pos, vec3 hitbox1, vec3 hitbox2):
position(pos), box1(hitbox1), box2(hitbox2-vec3(1,1,1))
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

void Entity::calc_constraints(World* world) {
    float axis_gap = 0.2;
    
    vec3 coords_array[3] = {{0,1,1}, {1,0,1}, {1,1,0}};
    vec3 dir_array[3] =    {{1,0,0}, {0,1,0}, {0,0,1}};
    
    vec3 newpos(position);
    
    bool new_consts[7];
    
    //box2 = vec3(0,0,0);
    
    //this system works by creating a slice of blocks that define each side of collision. the sides all share either the most positive point or the most negative point
    //the shlice is tested to see if any blocks are int it (2) and if there are blocks, the players location is rounded to outside the block (3)
    //axis gap make sure a side detects the collision of blocks head on before the side detectors sense the blocks. Each slice is moved out from the player by axis_gap
    //cout << "----------------------------------" << endl;
    //print(position);
    for (int axis = 0; axis < 3; axis ++) {
        vec3 coords = coords_array[axis];
        vec3 dir = dir_array[axis];
        
        // pos is the positive point of the hitbox rounded up to the nearest block. neg is the negative most point rounded down
        vec3 neg = floor( position + box1 - axis_gap*dir );// + vec3(0,1,0);
        vec3 pos = ceil( position + box2 + axis_gap*dir );
        
        
        
        //positive side
        //pos2 is the other point for the face
        vec3 pos2 = pos*dir + neg*coords;
        bool constraint = false;
        for (int x = (int)pos2.x; x < pos.x+1; x ++) {
            for (int y = (int)pos2.y; y < pos.y+1; y ++) {
                for (int z = (int)pos2.z; z < pos.z+1; z ++) {
                    Block* pix = world->get_global(x,y,z,1);
                    constraint = constraint or (pix != NULL and pix->get() != 0);
                }
            }
        }
        new_consts[axis] = constraint;
        if (constraint) {
            newpos[axis] = floor(newpos[axis] + box2[axis] + axis_gap) - box2[axis] - axis_gap;
        }
        vec3 neg2 = neg*dir + pos*coords;
        constraint = false;
        for (int x = (int)neg.x; x < neg2.x+1; x ++) {
            for (int y = (int)neg.y; y < neg2.y+1; y ++) {
                for (int z = (int)neg.z; z < neg2.z+1; z ++) {
                    Block* pix = world->get_global(x,y,z,1);
                    constraint = constraint or (pix != NULL and pix->get() != 0);
                }
            }
        }
        new_consts[axis+3] = constraint;
        if (constraint) {
            newpos[axis] = ceil(newpos[axis] + box1[axis] - axis_gap) - box1[axis] + axis_gap;
            
        }
    }
    /// torso check: to tell if the constraints are only on the feet
    
    vec3 neg = floor( position + box1 - axis_gap + vec3(0,1.2,0)) ;
    vec3 pos = ceil( position + box2 + axis_gap );
    bool constraint = false;
    for (int x = (int)neg.x; x < pos.x+1; x ++) {
        for (int y = (int)neg.y; y < pos.y+1; y ++) {
            for (int z = (int)neg.z; z < pos.z+1; z ++) {
                Block* pix = world->get_global(x,y,z,1);
                constraint = constraint or (pix != NULL and pix->get() != 0);
            }
        }
    }
    new_consts[6] = constraint;
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
    if (immunity) {
      immunity = false;
      return;
    }
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

#endif
