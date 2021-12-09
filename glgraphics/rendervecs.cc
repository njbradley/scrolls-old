#include "rendervecs.h"
#include "scrolls/ui.h"

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
  
  synclock.unlock();
  return RenderIndex(pos);
}

void AsyncGLVecs::del(RenderIndex index) {
  if (ignore) return;
  synclock.lock();
  
  emptys.emplace(index.index);
  RenderData empty;
  empty.pos.data[5] = NAN;
  changes[index.index] = empty;
  
  synclock.unlock();
}

void AsyncGLVecs::edit(RenderIndex index, RenderData data) { // ERR: heap error from changes[...], heap modified, called in tick thread
  if (ignore) return;
  synclock.lock();
  
  changes[index.index] = data;
  
  synclock.unlock();
}

void AsyncGLVecs::edit(RenderIndex index, RenderPosData posdata) {
  if (ignore) return;
  synclock.lock();
  
  changes[index.index].pos = posdata;
  
  synclock.unlock();
}

void AsyncGLVecs::sync() {
  synclock.lock();
  
  for (std::unordered_map<int,RenderData>::iterator change = changes.begin(); change != changes.end(); change ++) {
    destination->write(change->first + offset, change->second);
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
  // cout << "writing to " << endl;
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferSubData(GL_ARRAY_BUFFER, index.vert_index()*sizeof(GLfloat), index.vert_size*sizeof(GLfloat), data.pos.data);
  if (!data.type.isnull()) {
    glBindBuffer(GL_ARRAY_BUFFER, databuffer);
    glBufferSubData(GL_ARRAY_BUFFER, index.data_index()*sizeof(GLint), index.data_size*sizeof(GLint), data.type.data);
  }
}






void GLUIVecs::add(UIRect rect) {
  num_verts += 6;
  
  if (atlas != nullptr) {
    UIRect atlas_rect = atlas[rect.id];
    rect.uvpos = atlas_rect.uvpos + vec2(rect.uvpos.y, rect.uvpos.x) * atlas_rect.uvsize;
    rect.uvsize = atlas_rect.uvsize * vec2(rect.uvsize.y, rect.uvsize.x);
    rect.id = atlas_rect.id;
  }
  // rect.uvpos = vec2(0,0);
  // rect.uvsize = vec2(1,1);
  
  
  GLfloat id;
  *(GLint*)&id = rect.id;
  
  GLfloat x1 = rect.position.x;
  GLfloat y1 = rect.position.y;
  GLfloat x2 = rect.position.x + rect.size.x;
  GLfloat y2 = rect.position.y + rect.size.y;
  
  GLfloat uvx1 = rect.uvpos.x;
  GLfloat uvy1 = rect.uvpos.y;
  GLfloat uvx2 = rect.uvpos.x + rect.uvsize.x;
  GLfloat uvy2 = rect.uvpos.y + rect.uvsize.y;
  
  
  GLfloat newdata[] = {
    id, x1, y1, uvx2, uvy1,
    id, x2, y1, uvx2, uvy2,
    id, x2, y2, uvx1, uvy2,
    
    id, x1, y1, uvx2, uvy1,
    id, x2, y2, uvx1, uvy2,
    id, x1, y2, uvx1, uvy1,
  };
  
  datavec.insert(datavec.end(), newdata, newdata + 30);
}

void GLUIVecs::clear() {
  datavec.clear();
  num_verts = 0;
}


void* GLUIVecs::data() {
  return (void*) &datavec.front();
}

EXPORT_PLUGIN(AsyncGLVecs);
EXPORT_PLUGIN(GLUIVecs);
