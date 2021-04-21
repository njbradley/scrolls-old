#ifndef RENDERVEC
#define RENDERVEC

#include <ctime>
#include "rendervec.h"
#include "game.h"



RenderIndex::RenderIndex(int newindex): index(newindex) {
  
}

RenderIndex::RenderIndex(): index(-1) {
  
}

int RenderIndex::to_vert_index(int num) {
  return num*6;
}

int RenderIndex::to_data_index(int num) {
  return num*12;
}

int RenderIndex::vert_index() const {
  return to_vert_index(index);
}

int RenderIndex::data_index() const {
  return to_data_index(index);
}

bool RenderIndex::isnull() const {
  return index == -1;
}

const RenderIndex RenderIndex::npos(-1);


void AsyncGLVecs::set_destination(GLVecsDestination* dest) {
  destination = dest;
}

void AsyncGLVecs::set_destination_offset(GLVecsDestination* dest, int newoffset) {
  destination = dest;
  offset = newoffset;
}

RenderIndex AsyncGLVecs::add(RenderData data) {
  if (ignore) return RenderIndex::npos;
  synclock.lock();
  
  int pos;
  if (emptys.size() > 0) {
    pos = *emptys.begin();
    changes[pos] = data;
    emptys.erase(pos);
  } else {
    pos = num_verts;
    changes[num_verts++] = data;
  }
  
  synclock.unlock_shared();
  return RenderIndex(pos);
}

void AsyncGLVecs::del(RenderIndex index) {
  if (ignore) return;
  synclock.lock();
  
  emptys.emplace(index.index);
  RenderData empty;
  empty.pos.data[0] = NAN;
  changes[index.index] = empty;
  
  synclock.unlock();
}

void AsyncGLVecs::edit(RenderIndex index, RenderData data) {
  if (ignore) return;
  synclock.lock_shared();
  
  changes[index.index] = data;
  
  synclock.unlock_shared();
}

void AsyncGLVecs::sync() {
  synclock.lock();
  
  for (std::unordered_map<int,RenderData>::iterator change = changes.begin(); change != changes.end(); change ++) {
    destination->write(change->first, change->second);
  }
  
  addlock.lock();
  changes.clear();
  addlock.unlock();
  
  synclock.unlock();
}
  

void AsyncGLVecs::status(std::ostream& ofile) {
  ofile << "size:" << num_verts << endl;
  ofile << "changes:" << changes.size() << " emptys:" << emptys.size() << endl;
}






void GLVecsDestination::set_buffers(GLuint verts, GLuint databuf, int start_size) {
  vertexbuffer = verts;
  databuffer = databuf;
  size_alloc = start_size;
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, RenderIndex::to_vert_index(start_size)*sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, databuffer);
  glBufferData(GL_ARRAY_BUFFER, RenderIndex::to_data_index(start_size)*sizeof(GLint), NULL, GL_DYNAMIC_DRAW);
}


void GLVecsDestination::write(RenderIndex index, RenderData data) {
  //cout << index.index << ' ' << index.vert_index() << endl;
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferSubData(GL_ARRAY_BUFFER, index.vert_index()*sizeof(GLfloat), index.vert_size*sizeof(GLfloat), data.pos.data);
  glBindBuffer(GL_ARRAY_BUFFER, databuffer);
  glBufferSubData(GL_ARRAY_BUFFER, index.data_index()*sizeof(GLint), index.data_size*sizeof(GLint), data.type.data);
}


#endif
