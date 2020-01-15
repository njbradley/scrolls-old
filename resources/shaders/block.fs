#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;
in float lightLevel;
flat in int material;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
const int num_blocks = 8;
uniform sampler2D myTextureSampler[num_blocks];

void main(){

	// Output color = color of the texture at the specified UV
	for( int i = 0; i < num_blocks; i ++) {
		if (material == i) {
			color = texture( myTextureSampler[i], UV ).bgr * lightLevel;
			break;
		}
	}
}