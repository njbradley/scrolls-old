#ifndef RENDERVEC
#define RENDERVEC

#include <ctime>
#include "rendervecs.h"
#include "ui.h"

DEFINE_PLUGIN(RenderVecs);
DEFINE_PLUGIN(UIVecs);

RenderIndex::RenderIndex(int newindex): index(newindex) {
  
}

RenderIndex::RenderIndex(): index(-1) {
  
}

int RenderIndex::to_vert_index(int num) {
  return num*8;
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



void UIVecs::add(UIObj* obj) {
	obj->render(this);
}



#endif
