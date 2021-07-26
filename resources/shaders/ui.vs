#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec2 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in int mat;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
flat out int material;

void main() {
	gl_Position =  vec4(vertexPosition_modelspace,0,1);
	
	// UV of the vertex. No special space for this one.
	UV = vertexUV;
	material = mat;
}
