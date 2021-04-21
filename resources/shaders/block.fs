#version 330 core

in vec2 UV;
flat in uint tex;
flat in uint rot;
flat in uint edges;
in vec2 outlight;

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

void main() {
	
	vec4 tex = texture(myTextureSampler[0], vec3(UV.x, UV.y, tex));
	if (tex.a < 0.05) {
		discard;
	}
	vec3 color3 = tex.bgr * max(outlight.y, sunlight*outlight.x);
	color = vec4(color3.x, color3.y, color3.z, tex.a);
	/*
	const vec4 colors[] = {
		vec4(1,0,0,1),
		vec4(0,1,0,1),
		vec4(0,0,1,1),
		vec4(1,1,0,1),
		vec4(0,1,1,1),
		vec4(1,1,1,1)
	};
	
	color = colors[material];
	
	
	if (true) {
		vec4 tex = texture(myTextureSampler[0], vec3(UV.x, UV.y, material));
		
		uint edge1 = edges & 0xff;
		if (edge1 != 0) {
			vec4 added = texture(edgesTex, vec3(UV.x, UV.y, edge1-1));
			tex.rgb = tex.rgb * (1-added.a) + added.rgb * added.a;
		}
		uint edge2 = (edges & 0xff00) >> 8;
		if (edge2 != 0) {
			vec4 added = texture(edgesTex, vec3(1-UV.y, UV.x, edge2-1));
			tex.rgb = tex.rgb * (1-added.a) + added.rgb * added.a;
		}
		uint edge3 = (edges & 0xff0000) >> 16;
		if (edge3 != 0) {
			vec4 added = texture(edgesTex, vec3(1-UV.x, 1-UV.y, edge3-1));
			tex.rgb = tex.rgb * (1-added.a) + added.rgb * added.a;
		}
		uint edge4 = (edges & 0xff000000) >> 24;
		if (edge4 != 0) {
			vec4 added = texture(edgesTex, vec3(UV.y, 1-UV.x, edge4-1));
			tex.rgb = tex.rgb * (1-added.a) + added.rgb * added.a;
		}
		
		
		if (tex.a < 0.05) {
			discard;
		}
		vec3 color3 = tex.bgr * max(outlight.y, sunlight*outlight.x);
		//color3 = (color3*(1-dist) + clear_color*dist);
		color = vec4(color3.x, color3.y, color3.z, tex.a);
	}*/
}
