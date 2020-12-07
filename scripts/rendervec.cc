#ifndef RENDERVEC
#define RENDERVEC

#include <ctime>
#include "rendervec.h"
#include "game.h"


RenderIndex::RenderIndex(int newstart, int newsize): start(newstart), size(newsize) {
  
}

RenderIndex::RenderIndex(pair<int,int> pairindex): start(pairindex.first), size(pairindex.second) {
  
}

RenderIndex::RenderIndex(): start(-1), size(0) {
  
}

int RenderIndex::to_vert_index(int num) {
  return num*3;
}

int RenderIndex::to_uv_index(int num) {
  return num*2;
}

int RenderIndex::to_light_index(int num) {
  return num*2;
}

int RenderIndex::to_mat_index(int num) {
  return num*2;
}

int RenderIndex::vert_start() const {
  return to_vert_index(start);
}

int RenderIndex::uv_start() const {
  return to_uv_index(start);
}

int RenderIndex::light_start() const {
  return to_light_index(start);
}

int RenderIndex::mat_start() const {
  return to_mat_index(start);
}

int RenderIndex::vert_size() const {
  return to_vert_index(size);
}

int RenderIndex::uv_size() const {
  return to_uv_index(size);
}

int RenderIndex::light_size() const {
  return to_light_index(size);
}

int RenderIndex::mat_size() const {
  return to_mat_index(size);
}

int RenderIndex::vert_end() const {
  return vert_start() + to_vert_index(size);
}

int RenderIndex::uv_end() const {
  return uv_start() + to_uv_index(size);
}

int RenderIndex::light_end() const {
  return light_start() + to_light_index(size);
}

int RenderIndex::mat_end() const {
  return mat_start() + to_mat_index(size);
}

int RenderIndex::end() const {
  return start + size;
}

bool RenderIndex::isnull() const {
  return start == -1 and size == 0;
}

bool RenderIndex::can_add(RenderIndex other) const {
  return (start <= other.start and end() > other.start)
  or (other.start <= start and other.end() > start);
}

RenderIndex RenderIndex::add(RenderIndex other) {
  return RenderIndex(std::min(start, other.start), std::max(end(), other.end()));
}

const RenderIndex RenderIndex::npos(-1,0);


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

RenderIndex MemVecs::add(MemVecs* newvecs) {
  verts.insert(verts.end(), newvecs->verts.begin(), newvecs->verts.end());
  uvs.insert(uvs.end(), newvecs->uvs.begin(), newvecs->uvs.end());
  light.insert(light.end(), newvecs->light.begin(), newvecs->light.end());
  mats.insert(mats.end(), newvecs->mats.begin(), newvecs->mats.end());
  num_verts += newvecs->num_verts;
  if ( verts.size()/3 != num_verts or uvs.size()/2 != num_verts or light.size()/2 != num_verts or mats.size()/2 != num_verts) {
    cout << "ERR: unbalanced vectors in MemVecs::add" << endl;
    game->crash(3928657839029036);
  }
  return RenderIndex((num_verts - newvecs->num_verts), newvecs->num_verts);
}

void MemVecs::del(RenderIndex index) {
  verts.erase(verts.begin()+index.vert_start(), verts.begin()+index.vert_end());
  uvs.erase(uvs.begin()+index.uv_start(), uvs.begin()+index.uv_end());
  light.erase(light.begin()+index.light_start(), light.begin()+index.light_end());
  mats.erase(mats.begin()+index.mat_start(), mats.begin()+index.mat_end());
}
  

void MemVecs::clear() {
  verts.clear();
  uvs.clear();
  light.clear();
  mats.clear();
  num_verts = 0;
}

void MemVecs::edit(RenderIndex index, MemVecs* newvecs) {
  verts.insert(verts.begin()+index.vert_start(), newvecs->verts.begin(), newvecs->verts.end());
  uvs.insert(uvs.begin()+index.uv_start(), newvecs->uvs.begin(), newvecs->uvs.end());
  light.insert(light.begin()+index.light_start(), newvecs->light.begin(), newvecs->light.end());
  mats.insert(mats.begin()+index.mat_start(), newvecs->mats.begin(), newvecs->mats.end());
}






/*
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
  return pair<int,int>(-1,0);
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
}*/


AsyncGLVecs::Change::Change(RenderIndex newindex): index(newindex) {
  
}

AsyncGLVecs::Change::Change(RenderIndex newindex, MemVecs newvecs): index(newindex), vecs(newvecs) {
  
}


void AsyncGLVecs::set_destination(GLVecsDestination* dest) {
  destination = dest;
}

void AsyncGLVecs::set_destination_offset(GLVecsDestination* dest, int newoffset) {
  destination = dest;
  offset = newoffset;
}

RenderIndex AsyncGLVecs::add(MemVecs* vecs) {
  synclock.lock_shared();
  
  for (std::list<Empty>::iterator empty = emptys.begin(); empty != emptys.end(); empty ++) {
    if (empty->index.size > vecs->num_verts) {
      addlock.lock();
      RenderIndex result(empty->index.start, vecs->num_verts);
      changes.push_back({result, *vecs});
      addlock.unlock();
      empty->index = RenderIndex(empty->index.start + vecs->num_verts, empty->index.size - vecs->num_verts);
      if (empty->change != changes.end()) {
        empty->change->index = empty->index;
      }
      synclock.unlock_shared();
      return result;
    } else if (empty->index.size == vecs->num_verts) {
      RenderIndex result(empty->index.start, vecs->num_verts);
      if (empty->change == changes.end()) {
        addlock.lock();
        changes.push_back({result, *vecs});
        addlock.unlock();
      } else {
        empty->change->vecs.add(vecs);
      }
      emptys.erase(empty);
      synclock.unlock_shared();
      return result;
    }
  }
  
  addlock.lock();
  RenderIndex result(num_verts, vecs->num_verts);
  changes.push_back({result, *vecs});
  num_verts += vecs->num_verts;
  addlock.unlock();
  
  synclock.unlock_shared();
  return result;
}

void AsyncGLVecs::del(RenderIndex index) {
  synclock.lock_shared();
  
  for (std::list<Empty>::iterator empty = emptys.begin(); empty != emptys.end(); empty ++) {
    if (index.can_add(empty->index)) {
      empty->index = index.add(empty->index);
      if (empty->change != changes.end()) {
        empty->change->index = empty->index;
      }
      synclock.unlock_shared();
      return;
    }
  }
  
  addlock.lock();
  changes.emplace_back(index);
  std::list<Change>::iterator newchange = changes.begin();
  for (int i = 0; i < changes.size()-1; i ++) {
    newchange ++;
  }
  addlock.unlock();
  
  emptyaddlock.lock();
  emptys.push_back({index, newchange});
  emptyaddlock.unlock();
  
  synclock.unlock_shared();
}

void AsyncGLVecs::edit(RenderIndex index, MemVecs* vecs) {
  synclock.lock_shared();
  
  addlock.lock();
  changes.push_back({index, *vecs});
  addlock.unlock();
  
  synclock.unlock_shared();
}

void AsyncGLVecs::sync() {
  synclock.lock();
  
  for (std::list<Change>::iterator change = changes.begin(); change != changes.end(); change ++) {
    if (change->vecs.num_verts == 0) {
      change->vecs.verts.resize(RenderIndex::to_vert_index(change->index.size));
      change->vecs.uvs.resize(RenderIndex::to_uv_index(change->index.size));
      change->vecs.light.resize(RenderIndex::to_light_index(change->index.size));
      change->vecs.mats.resize(RenderIndex::to_mat_index(change->index.size));
    }
    
    RenderIndex writeindex = change->index;
    writeindex.start += offset;
    destination->write(writeindex, &change->vecs);
  }
  
  addlock.lock();
  changes.clear();
  addlock.unlock();
  
  for (std::list<Empty>::iterator empty = emptys.begin(); empty != emptys.end(); empty ++) {
    empty->change = changes.end();
  }
  
  synclock.unlock();
}
  

void AsyncGLVecs::status(std::ostream& ofile) {
  ofile << "size:" << num_verts << endl;
  ofile << "changes:" << changes.size() << " emptys:" << emptys.size() << endl;
}






void GLVecsDestination::set_buffers(GLuint verts, GLuint uvs, GLuint light, GLuint mats, int start_size) {
  vertexbuffer = verts;
  uvbuffer = uvs;
  lightbuffer = light;
  matbuffer = mats;
  size_alloc = start_size;
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, RenderIndex::to_vert_index(start_size)*sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, RenderIndex::to_uv_index(start_size)*sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, lightbuffer);
  glBufferData(GL_ARRAY_BUFFER, RenderIndex::to_light_index(start_size)*sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, matbuffer);
  glBufferData(GL_ARRAY_BUFFER, RenderIndex::to_mat_index(start_size)*sizeof(GLint), NULL, GL_DYNAMIC_DRAW);
  
  //cout << "err: " << std::hex << glGetError() << std::dec << endl;
}


void GLVecsDestination::write(RenderIndex index, MemVecs* vecs) {
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferSubData(GL_ARRAY_BUFFER, index.vert_start()*sizeof(GLfloat), vecs->verts.size()*sizeof(GLfloat), &vecs->verts.front());
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glBufferSubData(GL_ARRAY_BUFFER, index.uv_start()*sizeof(GLfloat), vecs->uvs.size()*sizeof(GLfloat), &vecs->uvs.front());
  glBindBuffer(GL_ARRAY_BUFFER, lightbuffer);
  glBufferSubData(GL_ARRAY_BUFFER, index.light_start()*sizeof(GLfloat), vecs->light.size()*sizeof(GLfloat), &vecs->light.front());
  glBindBuffer(GL_ARRAY_BUFFER, matbuffer);
  glBufferSubData(GL_ARRAY_BUFFER, index.mat_start()*sizeof(GLint), vecs->mats.size()*sizeof(GLfloat), &vecs->mats.front());
}


#endif
