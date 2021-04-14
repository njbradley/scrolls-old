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


void MemVecs::add_face(GLfloat* newverts, GLfloat* newuvs, GLfloat sunlight,
GLfloat blocklight, GLint minscale, GLint breaking, GLint overlay, GLint edges[4], GLint mat) {
    verts.insert(verts.end(), newverts, newverts+6*3);
    uvs.insert(uvs.end(), newuvs, newuvs+6*2);
    int matx = edges[3];
    matx *= 32;
    matx += edges[2];
    matx *= 32;
    matx += edges[1];
    matx *= 32;
    matx += edges[0];
    matx *= 64;
    matx += breaking;
    matx *= 64;
    matx += minscale;
    for (int i = 0; i < 6; i ++) {
        light.push_back(sunlight);
        light.push_back(blocklight);
        mats.push_back(matx);
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
  if (ignore) return RenderIndex::npos;
  synclock.lock();
  
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
  
  synclock.unlock();
  return result;
}

void AsyncGLVecs::del(RenderIndex index) {
  if (ignore) return;
  synclock.lock();
  
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
  
  synclock.unlock();
}

void AsyncGLVecs::edit(RenderIndex index, MemVecs* vecs) {
  if (ignore) return;
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
