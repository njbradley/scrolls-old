#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

flat in uvec2 facepx[];
flat in uvec2 facepy[];
flat in uvec2 facepz[];
flat in uvec2 facenx[];
flat in uvec2 faceny[];
flat in uvec2 facenz[];

flat in vec2 rotation[];
flat in float scale[];

out vec2 UV;
flat out uint tex;
flat out uint rot;
flat out uint edges;
out vec2 outlight;

uniform mat4 MVP;

vec3 sunlightDir = vec3(0,-1,0);

void set_attr(uvec2 data, vec4 normal) {
	edges = 0u;
	tex = data.x;
	rot = (data.y & 0xff00u) >> 8u;
	float sundot = dot(sunlightDir, normal.xyz/scale[0]);
	outlight = vec2(float((data.y & 0xff000000u) >> 24u) - (sundot*2+2), float((data.y & 0xff0000u) >> 16u)) / 20;
}

bool isCulled(vec4 normal, vec4 center) {
	// vec4 newnorm = (center + normal) / (center.w+normal.w) - center/center.w;
  return normal.z > 0;
}

void AddQuad(vec4 center, vec4 dy, vec4 dx, vec4 normal, uvec2 data) {
	/*
	vec2 point1 = (center.xy + dy.xy) / (center.w + dy.w);
	vec2 point2 = (center.xy + dx.xy) / (center.w + dx.w);
	vec2 cent = center.xy/center.w;
	float result = (point2.x - cent.x)*(point1.y - cent.y) - (point2.y - cent.y)*(point1.x - cent.x);
	if (center.w > max(max(-dx.w, -dy.w), max(-dx.w-dy.w, 0)) && result > 0) {
		return;
	}*/
	
	vec3 realnorm = abs(cross(dx.xyz, dy.xyz)) * sign(normal.xyz);
	float dotresult = dot(center.xyz, realnorm);
	if (dotresult > 0) {
		return;
	}
	
  UV = vec2(1,0);
	set_attr(data, normal);
  gl_Position = center + (dx );
  EmitVertex();
	
	UV = vec2(0,0);
	set_attr(data, normal);
	gl_Position = center;
  EmitVertex();

	UV = vec2(1,1);
	set_attr(data, normal);
	gl_Position = center + (dx + dy);
  EmitVertex();

	UV = vec2(0,1);
	set_attr(data, normal);
  gl_Position = center + ( dy);
  EmitVertex();

  EndPrimitive();
}



void main() {
	if (isnan(scale[0])) {
		return;
	}
	
  vec4 center = MVP * gl_in[0].gl_Position;
	float voxSize = scale[0];
  
	// vec4 centercam = (center) / center.w;
	// vec4 centercamhigh = (center + voxSize) / center.w;
	// if (centercamhigh.x < -1 || centercam.x > 1 || centercamhigh.y < -1 || centercam.y > 1) {
		// return;
	// }
	
	float divisor = center.w + voxSize;
	if (center.w < -voxSize || abs(max(center.x/divisor, center.y/divisor)) > 1 + 1.5*voxSize/divisor) {
		return;
	}
	
	sunlightDir = vec3(MVP * vec4(sunlightDir, 0));
	
	
  vec4 dx = MVP[0] * voxSize;
  vec4 dy = MVP[1] * voxSize;
  vec4 dz = MVP[2] * voxSize;
	
	/*
	uvec2 chosenface;
	if ((facepx[0].x) != 0u) {
  	chosenface = facepx[0];
  }
  if ((facenx[0].x) != 0u) {
    chosenface = facenx[0];
	}
  if ((facepy[0].x) != 0u) {
    chosenface = facepy[0];
	}
  if ((faceny[0].x) != 0u) {
    chosenface = faceny[0];
	}
  if ((facepz[0].x) != 0u) {
    chosenface = facepz[0];
	}
  if ((facenz[0].x) != 0u) {
    chosenface = facenz[0];
	}
	
	AddQuad(center, vec4(voxSize,0,0,0), vec4(0,voxSize,0,0), vec4(0,0,-voxSize,0), chosenface);
	return;*/
	
	if ((facepx[0].x) != 0u) {
  	AddQuad(center + dx, dy, dz,  dx, facepx[0]);
  }
  if ((facenx[0].x) != 0u) {
    AddQuad(center, dz, dy,  -dx, facenx[0]);
	}
  if ((facepy[0].x) != 0u) {
    AddQuad(center + dy, dz, dx,  dy, facepy[0]);
	}
  if ((faceny[0].x) != 0u) {
    AddQuad(center, dx, dz,  -dy, faceny[0]);
	}
  if ((facepz[0].x) != 0u) {
    AddQuad(center + dz, dx, dy,  dz, facepz[0]);
	}
  if ((facenz[0].x) != 0u) {
    AddQuad(center, dy, dx,  -dz, facenz[0]);
	}
}
