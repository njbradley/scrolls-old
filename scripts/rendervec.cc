#ifndef RENDERVEC
#define RENDERVEC

#include <ctime>
#include "rendervec.h"
#include "game.h"



void MemVecs::add_face(GLfloat* newverts, GLfloat* newuvs, GLfloat sunlight, GLfloat blocklight, GLint minscale, GLint mat) {
    verts.insert(verts.end(), newverts, newverts+6*3);
    uvs.insert(uvs.end(), newuvs, newuvs+6*2);
    for (int i = 0; i < 6; i ++) {
        light.push_back(sunlight);
        light.push_back(blocklight);
        mats.push_back(minscale);
        mats.push_back(mat);
    }
    num_verts += 6;
    if ( verts.size()/3 != num_verts or uvs.size()/2 != num_verts or light.size()/2 != num_verts or mats.size()/2 != num_verts) {
      cout << "ERR: unbalanced vectors in MemVecs::add_face" << endl;
      game->crash(8327490283652347);
    }
}

pair<int,int> MemVecs::add(MemVecs* newvecs) {
  verts.insert(verts.end(), newvecs->verts.begin(), newvecs->verts.end());
  uvs.insert(uvs.end(), newvecs->uvs.begin(), newvecs->uvs.end());
  light.insert(light.end(), newvecs->light.begin(), newvecs->light.end());
  mats.insert(mats.end(), newvecs->mats.begin(), newvecs->mats.end());
  num_verts += newvecs->num_verts;
  if ( verts.size()/3 != num_verts or uvs.size()/2 != num_verts or light.size()/2 != num_verts or mats.size()/2 != num_verts) {
    cout << "ERR: unbalanced vectors in MemVecs::add" << endl;
    game->crash(3928657839029036);
  }
  return pair<int,int>((num_verts - newvecs->num_verts)/6, newvecs->num_verts/6);
}

void MemVecs::del(pair<int,int> index) {
  int start = index.first*6;
  int end = start + index.second*6;
  verts.erase(verts.begin()+start*3, verts.begin()+end*3);
  uvs.erase(uvs.begin()+start*2, uvs.begin()+end*2);
  light.erase(light.begin()+start*2, light.begin()+end*2);
  mats.erase(mats.begin()+start*2, mats.begin()+end*2);
}
  

void MemVecs::clear() {
  verts.clear();
  uvs.clear();
  light.clear();
  mats.clear();
  num_verts = 0;
}

void MemVecs::edit(pair<int,int> index, MemVecs* newvecs) {
  int start = index.first;
  verts.insert(verts.begin()+start*3, newvecs->verts.begin(), newvecs->verts.end());
  uvs.insert(uvs.begin()+start*2, newvecs->uvs.begin(), newvecs->uvs.end());
  light.insert(light.begin()+start*2, newvecs->light.begin(), newvecs->light.end());
  mats.insert(mats.begin()+start*2, newvecs->mats.begin(), newvecs->mats.end());
  
}







void GLVecs::set_buffers(GLuint verts, GLuint uvs, GLuint light, GLuint mats, int start_size) {
    vertexbuffer = verts;
    uvbuffer = uvs;
    lightbuffer = light;
    matbuffer = mats;
    size_alloc = start_size;
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, start_size*3*sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, start_size*2*sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, lightbuffer);
    glBufferData(GL_ARRAY_BUFFER, start_size*2*sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, matbuffer);
    glBufferData(GL_ARRAY_BUFFER, start_size*2*sizeof(GLint), NULL, GL_DYNAMIC_DRAW);
    
    //cout << "err: " << std::hex << glGetError() << std::dec << endl;
}

void GLVecs::set_buffers_prealloc(GLuint verts, GLuint uvs, GLuint light, GLuint mats, int start_size, int newoffset) {
    vertexbuffer = verts;
    uvbuffer = uvs;
    lightbuffer = light;
    matbuffer = mats;
    size_alloc = start_size;
    offset = newoffset;
}

pair<int,int> GLVecs::add(MemVecs* newvecs) {
  if (writelock.try_lock_for(std::chrono::seconds(1))) {
    //cout << 84 << endl;
    int start = clock();
    int old_num_verts = num_verts;
    int location = num_verts;
    
    for (int i = 0; i < empty.size(); i ++) {
        int num_faces = empty[i].second;
        if (num_faces >= newvecs->num_verts/6) {
            location = empty[i].first*6 + offset;
            pair<int,int> index(empty[i].first, newvecs->num_verts/6);
            pair<int,int> new_empty(index.first+index.second, empty[i].second - index.second);
            //cout << "used: " << empty[i].first << ' ' << empty[i].second << " for " << newvecs->num_verts/6 << endl;
            //cout << "returned " << index.first << ' ' << index.second << "empty left is " << new_empty.first << ' ' << new_empty.second << endl;
            if (new_empty.second <= 0) {
              //cout << "removing empty" << endl;
              empty.erase(empty.begin()+i);
            } else {
              empty[i] = new_empty;
            }
        
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
            glBufferSubData(GL_ARRAY_BUFFER, location*3*sizeof(GLfloat), newvecs->verts.size()*sizeof(GLfloat), &newvecs->verts.front());
            glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
            glBufferSubData(GL_ARRAY_BUFFER, location*2*sizeof(GLfloat), newvecs->uvs.size()*sizeof(GLfloat), &newvecs->uvs.front());
            glBindBuffer(GL_ARRAY_BUFFER, lightbuffer);
            glBufferSubData(GL_ARRAY_BUFFER, location*2*sizeof(GLfloat), newvecs->light.size()*sizeof(GLfloat), &newvecs->light.front());
            glBindBuffer(GL_ARRAY_BUFFER, matbuffer);
            glBufferSubData(GL_ARRAY_BUFFER, location*2*sizeof(GLint), newvecs->mats.size()*sizeof(GLfloat), &newvecs->mats.front());
        
            writelock.unlock();
            return index;
        }
    }
    if (num_verts+newvecs->num_verts > size_alloc) {
        writelock.unlock();
        game->crash(2);
        return pair<int,int>(-1,0);
    }
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferSubData(GL_ARRAY_BUFFER, num_verts*3*sizeof(GLfloat) + offset*3*sizeof(GLfloat), newvecs->verts.size()*sizeof(GLfloat), &newvecs->verts.front());
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferSubData(GL_ARRAY_BUFFER, num_verts*2*sizeof(GLfloat) + offset*2*sizeof(GLfloat), newvecs->uvs.size()*sizeof(GLfloat), &newvecs->uvs.front());
    glBindBuffer(GL_ARRAY_BUFFER, lightbuffer);
    glBufferSubData(GL_ARRAY_BUFFER, num_verts*2*sizeof(GLfloat) + offset*2*sizeof(GLfloat), newvecs->light.size()*sizeof(GLfloat), &newvecs->light.front());
    glBindBuffer(GL_ARRAY_BUFFER, matbuffer);
    glBufferSubData(GL_ARRAY_BUFFER, num_verts*2*sizeof(GLint) + offset*2*sizeof(GLfloat), newvecs->mats.size()*sizeof(GLfloat), &newvecs->mats.front());
    //cout << "err: " << std::hex << glGetError() << std::dec << endl;
    
    num_verts += newvecs->num_verts;
    writelock.unlock();
    return pair<int,int>(old_num_verts/6, newvecs->num_verts/6);
  }
}


void GLVecs::del(pair<int,int> index) {
  if (index.first == -1) {
    cout << "found " << endl;
  }
  if (writelock.try_lock_for(std::chrono::seconds(1))) {
    //cout << 142 << endl;
    if (index.first < -1) {
      cout << index.first << ' ' << index.second << endl;
      cout << "big problem" << endl;
      writelock.unlock();
      // game->crash(19782984921423);
      return;
    }
    
    //cout << "del: " << index.first << ' ' << index.second << endl;
    // if (index.second > 0) {
    //   int start = index.first*6;
    //   int size = index.second*6;
    //   GLfloat zerosf[size*3];
    //   memset(zerosf, 0.0f, size*3);
    //   for (int i = 0; i < size*3; i ++) {
    //     cout << zerosf[i] << endl;
    //   }
    //   glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    //   glBufferSubData(GL_ARRAY_BUFFER, start*3*sizeof(GLfloat), size*3*sizeof(GLfloat), zerosf);
    // }
    for (int i = 0; i < empty.size(); i ++) {
      if ((index.first <= empty[i].first and index.first+index.second > empty[i].first) or
        (empty[i].first <= index.first and empty[i].first+empty[i].second > index.first)) {
        if (index.second > 0) {
          cout << index.first << ' ' << index.second << ' ' << empty[i].first << ' '<< empty[i].second << " problem" << endl;
          writelock.unlock();
          game->crash(1567483904356848390);
        }
      }
      if (empty[i].first + empty[i].second == index.first) {
        //cout << "combined " << empty[i].first << ' ' << empty[i].second << ':' << empty[i].first + empty[i].second
        //  << " with " << index.first << ' ' << index.second << ':' << index.first + index.second;
        empty[i] = pair<int,int>(empty[i].first, empty[i].second + index.second);
        //cout << " result " << empty[i].first << ' ' << empty[i].second << endl;
        if (empty[i].first + empty[i].second == num_verts/6) {
          //cout << "empty " << empty[i].first << ' ' << empty[i].second << " merged out old num " << num_verts/6;
          num_verts -= empty[i].second*6;
          //cout << ' ' << num_verts/6 << endl;
          empty.erase(empty.begin()+i);
          //game->crash(1178656827093404);
        }
        clean_flag = true;
        writelock.unlock();
        return;
      }
      if (index.first + index.second == empty[i].first) {
        //cout << "combined " << empty[i].first << ' ' << empty[i].second << ':' << empty[i].first + empty[i].second
        //  << " with " << index.first << ' ' << index.second << ':' << index.first + index.second;
        empty[i] = pair<int,int>(index.first, index.second + empty[i].second);
        //cout << " result " << empty[i].first << ' ' << empty[i].second << endl;
        if (empty[i].first + empty[i].second == num_verts/6) {
          //cout << "empty " << empty[i].first << ' ' << empty[i].second << " merged out old num " << num_verts/6;
          num_verts -= empty[i].second*6;
          //cout << ' ' << num_verts/6 << endl;
          empty.erase(empty.begin()+i);
          //game->crash(829578937180496782);
        }
        clean_flag = true;
        writelock.unlock();
        return;
      }
    }
    if ( index.second != 0) {
        empty.push_back(index);
        clean_flag = true;
    }
    writelock.unlock();
  }
}


void GLVecs::clean() {
  //if (writelock.try_lock_for(std::chrono::seconds(1))) {
    //cout << 207 << endl;
    //int start = clock();
  if (clean_flag) {
    clean_flag = false;
    // cout << "cle" << offset << endl;
    // cout << empty.size() << endl;
    
		int max = 0;
    for (pair<int,int> index : empty) {
			if (index.second*6 > max) {
				max = index.second*6;
			}
		}
		vector<GLfloat> zerosf(max*3, 1.91f);
		vector<GLint> zerosi(max*2, 0);
    for (pair<int,int> index : empty) {
      int start = index.first*6 + offset;
      int size = index.second*6;
      if (writelock.try_lock_for(std::chrono::seconds(1))) {
        if (size > max) {
          zerosf.resize(size*3, 1.92f);
          zerosi.resize(size*2, 0);
          max = size;
        }
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferSubData(GL_ARRAY_BUFFER, start*3*sizeof(GLfloat), size*3*sizeof(GLfloat), &zerosf.front());
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glBufferSubData(GL_ARRAY_BUFFER, start*2*sizeof(GLfloat), size*2*sizeof(GLfloat), &zerosf.front());
        glBindBuffer(GL_ARRAY_BUFFER, lightbuffer);
        glBufferSubData(GL_ARRAY_BUFFER, start*2*sizeof(GLfloat), size*sizeof(GLfloat), &zerosf.front());
        glBindBuffer(GL_ARRAY_BUFFER, matbuffer);
        glBufferSubData(GL_ARRAY_BUFFER, start*2*sizeof(GLint), size*2*sizeof(GLint), &zerosi.front());
        writelock.unlock();
      }
    }
    // cout << "CLE" << offset << endl;
  }
  //  writelock.unlock();
    // int all = clock() - start;
    // if (all > 2) {
    //   cout << all << " clean" << endl;
    // }
  //}
}

void GLVecs::edit( pair<int,int> index, MemVecs* newvecs) {
  if (writelock.try_lock_for(std::chrono::seconds(1))) {
    //cout << 235 << endl;
    //cout << "editing!" << endl;
    int location = index.first*6 + offset;
    
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferSubData(GL_ARRAY_BUFFER, location*3*sizeof(GLfloat), newvecs->verts.size()*sizeof(GLfloat), &newvecs->verts.front());
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferSubData(GL_ARRAY_BUFFER, location*2*sizeof(GLfloat), newvecs->uvs.size()*sizeof(GLfloat), &newvecs->uvs.front());
    glBindBuffer(GL_ARRAY_BUFFER, lightbuffer);
    glBufferSubData(GL_ARRAY_BUFFER, location*2*sizeof(GLfloat), newvecs->light.size()*sizeof(GLfloat), &newvecs->light.front());
    glBindBuffer(GL_ARRAY_BUFFER, matbuffer);
    glBufferSubData(GL_ARRAY_BUFFER, location*2*sizeof(GLint), newvecs->mats.size()*sizeof(GLfloat), &newvecs->mats.front());
    
    writelock.unlock();
  }
}

void GLVecs::status(std::stringstream & message) {
    message << "-----memory vectors-----" << endl;
    message << "num verts: " << num_verts/6 << endl;
    message << "allocated: " << size_alloc/6 << endl;
    int sum = 0;
    for (pair<int,int> f : empty) {
        sum += f.second;
    }
    message << "empty: " << sum << " in " << empty.size() << "emptys" << endl;
}

#endif
