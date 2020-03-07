#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;
in float lightLevel;
flat in ivec2 material;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
const int num_blocks = 3;
uniform sampler2DArray myTextureSampler[num_blocks];

void main(){
	// Output color = color of the texture at the specified UV
	color = vec3(1,0,0);//vec3(material.x/3.0f, 0, material.y/10.0f);
	//return;
	for( int i = 0; i < num_blocks; i ++) {
		if (material.x-1 == i) {
			color = texture( myTextureSampler[i], vec3(UV.x, UV.y, material.y) ).bgr * lightLevel;
			break;
		}
	}
	
	//if (lightLevel == 0) {
	//	color = vec3(1,0,0);
	//}
}
