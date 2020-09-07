#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;
flat in int material;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
const int num_uis = 5;
uniform sampler2D myTextureSampler[num_uis];

void main(){

	// Output color = color of the texture at the specified UV
	
	if (material == 0) {
		color = texture( myTextureSampler[0], UV ).bgra;
	} else if (material == 1) {
		color = texture( myTextureSampler[1], UV ).bgra;
	} else if (material == 2) {
		color = texture( myTextureSampler[2], UV ).bgra;
	} else if (material == 3) {
		color = texture( myTextureSampler[3], UV ).bgra;
	} else if (material == 4) {
		color = texture( myTextureSampler[4], UV ).bgra;
	}
}
