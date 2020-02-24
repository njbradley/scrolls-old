// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <chrono>
#include <thread>
using std::stringstream;
using std::vector;
using std::string;
#include <fstream>
using std::ifstream;

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
#include "classes.h"


// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;


class Menu { public:
    virtual void render(GLFWwindow* window, World* world, Player* player, RenderVecs* uivecs) = 0;
};

class Inventory: public Menu { public:
    string header;
    pair<Item*,int> in_hand;
    function<void()> after;
    ItemContainer* other;
    int button;
    double xpos, ypos;
    
    Inventory(string head, ItemContainer* start_other, function<void()> after_func);
    void render(GLFWwindow* window, World* world, Player* player, RenderVecs* uivecs);
};

class Select : public Menu { public:
    string header;
    vector<string> options;
    string chosen;
    function<void(string)> after;
    
    Select(string head, vector<string> & opts, function<void(string)> after_func);
    
    void render(GLFWwindow* window, World* world, Player* player, RenderVecs* uivecs);
};
