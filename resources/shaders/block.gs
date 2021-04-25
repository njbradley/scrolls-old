#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 12) out;

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

uniform mat4 MVmat;
uniform mat4 Pmat;
uniform vec3 sunlightDir

float voxSize;

void set_attr(uvec2 data, vec4 normal) {
	edges = 0u;
	tex = data.x;
	rot = (data.y & 0xff00u) >> 8u;
	float sundot = dot(sunlightDir, normal.xyz/voxSize);
	outlight = vec2(float((data.y & 0xff000000u) >> 24u) - (sundot*2+2), float((data.y & 0xff0000u) >> 16u)) / 20;
}

void addQuad(vec4 position, vec4 dy, vec4 dx, vec4 normal, uvec2 data) {
	if (dot(position, normal) > 0) {
		return;
	}
	
  UV = vec2(1,0);
	set_attr(data, normal);
  gl_Position = Pmat * (position + dx);
  EmitVertex();
	
	UV = vec2(0,0);
	set_attr(data, normal);
	gl_Position = Pmat * position;
  EmitVertex();

	UV = vec2(1,1);
	set_attr(data, normal);
	gl_Position = Pmat * (position + dx + dy);
  EmitVertex();

	UV = vec2(0,1);
	set_attr(data, normal);
  gl_Position = Pmat * (position + dy);
  EmitVertex();

  EndPrimitive();
}


void main() {
	if (isnan(scale[0])) {
		return;
	}
	
  vec4 position = MVmat * gl_in[0].gl_Position;
	
	vec4 center = (position + MVmat * vec4(0.5,0.5,0.5,0));
	voxSize = scale[0];
	
	vec4 screencenter = Pmat * center;
	float divisor = screencenter.w + voxSize;
	if (position.z > 1.8*voxSize || abs(max(screencenter.x/divisor, screencenter.y/divisor)) > 1 + 1.8*voxSize/divisor) {
		return;
	}
	
  vec4 dx = MVmat[0] * voxSize;
  vec4 dy = MVmat[1] * voxSize;
  vec4 dz = MVmat[2] * voxSize;
	
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
	
	addQuad(center-vec4(voxSize/2,voxSize/2,0,0), vec4(voxSize,0,0,0), vec4(0,voxSize,0,0), vec4(0,0,voxSize,0), chosenface);
	return;*/
	
	int max = 0;
	
	
	if ((facepx[0].x) != 0u) {
  	addQuad(position + dx, dy, dz,  dx, facepx[0]);
  }
  if ((facenx[0].x) != 0u) {
    addQuad(position, dz, dy,  -dx, facenx[0]);
	}
  if ((facepy[0].x) != 0u) {
    addQuad(position + dy, dz, dx,  dy, facepy[0]);
	}
  if ((faceny[0].x) != 0u) {
    addQuad(position, dx, dz,  -dy, faceny[0]);
	}
  if ((facepz[0].x) != 0u) {
    addQuad(position + dz, dx, dy,  dz, facepz[0]);
	}
  if ((facenz[0].x) != 0u) {
    addQuad(position, dy, dx,  -dz, facenz[0]);
	}
}
