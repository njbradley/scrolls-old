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
uniform vec3 clear_color;
uniform int view_distance;

void main(){
	// Output color = color of the texture at the specified UV
	float dist = gl_FragCoord.z / gl_FragCoord.w;
	dist /= view_distance;
	if (dist > 1) {
		dist = 1;
	}
	dist = dist*dist;
	//return;
	for( int i = 0; i < num_blocks; i ++) {
		if (material.x-1 == i) {
			color = texture( myTextureSampler[i], vec3(UV.x, UV.y, material.y) ).bgr * lightLevel;
			color = (color*(1-dist) + clear_color*dist);
			break;
		}
	}
	
	//if (lightLevel == 0) {
	//	color = vec3(1,0,0);
	//}
}
