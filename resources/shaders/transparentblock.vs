#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec2 light;
layout(location = 3) in ivec2 mat;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
out vec2 lightLevel;
out float vertex_dist;
flat out ivec2 material;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform vec3 player_position;

void main(){

	// Output position of the vertex, in clip space : MVP * position
  
	vertex_dist = length(vertexPosition_modelspace - player_position);
	
	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	
	// UV of the vertex. No special space for this one.
	UV = vertexUV;
	lightLevel = light;
	material = mat;
}
