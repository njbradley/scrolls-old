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
	for( int i = 0; i < num_uis; i ++) {
		if (material == i) {
			color = texture( myTextureSampler[i], UV ).bgra;
			break;
		}
	}
}
