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
uniform sampler2DArray breakingTex;
uniform sampler2DArray overlayTex;
uniform sampler2DArray edgesTex;
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
	
	int matx = material.x;
	int minscale = matx % 64;
	matx /= 64;
	int breaking = matx % 64;
	matx /= 64;
	int edge1 = matx % 32;
	matx /= 32;
	int edge2 = matx % 32;
	matx /= 32;
	int edge3 = matx % 32;
	matx /= 32;
	int edge4 = matx % 32;
	
	if (minscale == 1) {
		vec4 tex = texture(myTextureSampler[0], vec3(UV.x, UV.y, material.y));
		
		// if (overlay != 0) {
		// 	vec4 added = texture(overlayTex, vec3(UV.x, UV.y, overlay-1));
		// 	if (breaking != 0) {
		// 		float breakval = texture(edgesTex, vec3(UV.x, UV.y, edges-1)).a;
		// 		added.a *= breakval;
		// 	}
		// 	if (edges != 0) {
		// 		float edgeval = texture(edgesTex, vec3(UV.x, UV.y, edges-1)).a;
		// 		added.a *= edgeval;
		// 	}
		// 	tex.rgb = tex.rgb * (1-added.a) + added.rgb * added.a;
		// }
		if (edge1 != 0) {
			vec4 added = texture(edgesTex, vec3(UV.x, UV.y, edge1-1));
			tex.rgb = tex.rgb * (1-added.a) + added.rgb * added.a;
		}
		if (edge2 != 0) {
			vec4 added = texture(edgesTex, vec3(1-UV.y, UV.x, edge2-1));
			tex.rgb = tex.rgb * (1-added.a) + added.rgb * added.a;
		}
		if (edge3 != 0) {
			vec4 added = texture(edgesTex, vec3(1-UV.x, 1-UV.y, edge3-1));
			tex.rgb = tex.rgb * (1-added.a) + added.rgb * added.a;
		}
		if (edge4 != 0) {
			vec4 added = texture(edgesTex, vec3(UV.y, 1-UV.x, edge4-1));
			tex.rgb = tex.rgb * (1-added.a) + added.rgb * added.a;
		}
		
		
		if (tex.a < 0.05) {
			discard;
		}
		vec3 color3 = tex.bgr * max(lightLevel.y, sunlight*lightLevel.x);
		color3 = (color3*(1-dist) + clear_color*dist);
		color = vec4(color3.x, color3.y, color3.z, tex.a);
	}
}
