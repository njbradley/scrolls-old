#include "graphics.h"
#include "rendervecs.h"


ViewBox::ViewBox() {
	params.view_distance = 1000;
	params.clear_color = vec3(1,1,0);
	params.sun_color = vec3(1,1,1);
	params.sun_direction = vec3(0,-1,0);
}

void ViewBox::timestep(double deltatime, double worldtime) {
	
}

Plugin<GraphicsContext> graphics;
