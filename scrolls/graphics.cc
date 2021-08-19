#include "graphics.h"
#include "libraries.h"

DEFINE_PLUGIN(ViewBox);
DEFINE_PLUGIN(GraphicsContext);

EXPORT_PLUGIN(ViewBox);

ViewBox::ViewBox() {
	suntexture = graphics->block_textures()->getindex("sun.bmp");
	moontexture = graphics->block_textures()->getindex("moon.bmp");
}

void ViewBox::timestep(double deltatime, double daytime) {
	
	const double min_sun = 0.5;
  
  const int sunrise = 450;
  const int sunset = 1550;
  const int riseduration = 100;
  
  float sunlevel;
  sunlevel = min_sun;
  if (daytime > sunrise and daytime < sunrise + riseduration) {
    sunlevel = float(daytime-sunrise)/riseduration * (1-min_sun) + min_sun;
  } else if (daytime >= sunrise + riseduration and daytime <= sunset - riseduration) {
    sunlevel = 1;
  } else if (daytime > sunset - riseduration and daytime < sunset) {
    sunlevel = -float(daytime-sunset+riseduration)/riseduration * (1-min_sun) + 1;
  }
  
  vec3 sundirection = vec3(sin(daytime*3.14/1000), -cos(daytime*3.14/1000), 0);
  
  RenderData data;
  data.pos.loc.pos = *playerpos + sundirection * 1000.0f;
  data.pos.loc.rot = quat(cos(daytime*3.14/1000/2), 0, 0, sin(daytime*3.14/1000/2));
  data.pos.loc.scale = 64;
  
  for (int i = 0; i < 6; i ++) {
    data.type.faces[i] = {(uint)suntexture, 0, 0, 20, 20};
  }
  
  if (sunindex.isnull()) {
    sunindex = graphics->blockvecs()->add(data);
  } else {
    graphics->blockvecs()->edit(sunindex, data);
  }
  
  data.pos.loc.pos = *playerpos - sundirection * 1000.0f;
  data.pos.loc.rot = quat(cos(daytime*3.14/1000/2 + 3.14), 0, 0, sin(daytime*3.14/1000/2 + 3.14));
  data.pos.loc.scale = 64;
  
  for (int i = 0; i < 6; i ++) {
    data.type.faces[i] = {(uint)moontexture, 0, 0, 20, 20};
  }
  
  if (moonindex.isnull()) {
    moonindex = graphics->blockvecs()->add(data);
  } else {
    graphics->blockvecs()->edit(moonindex, data);
  }
	
	vec3 suncolor;
  
  float eveningTime = (daytime < 1000 ? daytime - sunrise : sunset - daytime) / riseduration;
  vec3 skycolor;
  if (eveningTime > 1) {
    skycolor = vec3(0.4f, 0.7f, 1.0f);
    suncolor = vec3(1,1,1);
  } else if (eveningTime < 0) {
    skycolor = vec3(0, 0, 0);
    suncolor = vec3(0.6,0.8,1);
  } else {
    float lateEvening = std::min(eveningTime, 0.2f) * 5;
    skycolor = vec3(1 - eveningTime*0.6f, 0.2f + eveningTime*0.5f, eveningTime) * lateEvening;
    suncolor = vec3(1, 0.2f + eveningTime*0.8f, eveningTime) * lateEvening + vec3(0.6, 0.8, 1) * (1-lateEvening);
  }
  
	params.view_distance = 1000;
	params.clear_color = skycolor * sunlevel;
	params.sun_color = suncolor * sunlevel;
	params.sun_direction = sundirection;
}

Plugin<ViewBox> viewbox;
Plugin<GraphicsContext> graphics;
