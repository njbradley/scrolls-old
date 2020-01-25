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
            
        }
        
        void add(Item* item) {
            for (int i = 0; i < items.size(); i ++) {
                if (item == items[i].first) {
                    items[i].second ++;
                    return;
                }
            }
            items.push_back(pair<Item*,int>(item,1));
        }
        
        Item* get(int index) {
            if (index < items.size()) {
                return items[index].first;
            }
            return nullptr;
        }
        
        Item* take(int index) {
            items[index].second --;
            if (items[index].second <= 0) {
                
            }
        }
        
        void render(RenderVecs* vecs) {
			draw_image(vecs, "inven_select.bmp", -0.5f, -1, 1, 0.15f*aspect_ratio);
            for (int i = 0; i < items.size(); i ++) {
                draw_image_uv(vecs, "items.bmp", -0.5f + i*0.1f, -0.98f, 0.1f, 0.1f, items[i].first->texture/64.0f, (items[i].first->texture+1)/64.0f);
                draw_text(std::to_string(items[i].second), -0.48f + i*0.1f, -0.98f, vecs);
            }
        }
};
