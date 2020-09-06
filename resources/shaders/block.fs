#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;
in vec2 lightLevel;
flat in ivec2 material;
in float vertex_dist;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
const int num_blocks = 3;
uniform sampler2DArray myTextureSampler[num_blocks];
uniform vec3 clear_color;
uniform int view_distance;
uniform float sunlight;

void main(){
	// Output color = color of the texture at the specified UV
	//float dist = gl_FragCoord.z / gl_FragCoord.w;
	float dist = vertex_dist / view_distance;
	if (dist > 1) {
		dist = 1;
	}
	dist = dist*dist;
	//return;
	
	if (material.x == 1) {
		vec4 tex = texture( myTextureSampler[0], vec3(UV.x, UV.y, material.y) );
		if (tex.a < 0.05) {
			discard;
		}
		vec3 color3 = tex.bgr * max(lightLevel.y, sunlight*lightLevel.x);
		color3 = (color3*(1-dist) + clear_color*dist);
		color = vec4(color3.x, color3.y, color3.z, tex.a);
	}
	
	//if (lightLevel == 0) {
	//	color = vec3(1,0,0);
	//}
}
