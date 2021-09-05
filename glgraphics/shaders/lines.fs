#version 330 core

in vec3 linecolor;

out vec4 color;

void main() {
   color = vec4(linecolor, 1);
}
