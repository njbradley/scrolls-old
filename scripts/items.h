#ifndef ITEMS
#define ITEMS
#include <iostream>
using std::cout;
using std::endl;
#include <functional>
using std::function;
#include "ui.h"
#include "itemdata.h"






class ItemContainer {
    public:
        
        vector<pair<Item*,int> > items;
        int size;
        
        ItemContainer(int newsize): size(newsize) {
            for (int i = 0; i < newsize; i ++) {
                items.push_back(pair<Item*,int>(nullptr, 0));
            }
        }
        
        bool add(Item* item, int num) {
            for (int i = 0; i < items.size(); i ++) {
                if (item == items[i].first) {
                    items[i].second += num;
                    return true;
                }
            }
            for (int i = 0; i < items.size(); i ++) {
                if (items[i].first == nullptr) {
                    items[i] = pair<Item*, int>(item, num);
                    return true;
                }
            }
            return false;
            //items.push_back(pair<Item*,int>(item,num));
        }
        
        Item* get(int index) {
            if (index < items.size()) {
                return items[index].first;
            }
            return nullptr;
        }
        
        Item* use(int index) {
            if (index < items.size()) {
                Item* ret = items[index].first;
                items[index].second--;
                if (items[index].second <= 0) {
                  items[index] = pair<Item*,int>(nullptr,0);
                }
                return ret;
            }
            return nullptr;
        }
        
        void render(RenderVecs* vecs, float x, float y) {
            draw_image(vecs, "inven_select.bmp", x, y, 1, 0.1f*aspect_ratio);
            for (int i = 0; i < items.size(); i ++) {
                if (items[i].first != nullptr) {
                    draw_image_uv(vecs, "items.bmp", x + i*0.1f, y+0.02f, 0.1f, 0.1f*aspect_ratio, items[i].first->texture/64.0f, (items[i].first->texture+1)/64.0f);
                    draw_text(vecs, std::to_string(items[i].second), x+0.02 + i*0.1f, y+0.02f);
                }
            }
        }
};

#endif
