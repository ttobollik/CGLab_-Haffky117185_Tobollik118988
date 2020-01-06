#version 150

in  vec3 pass_Normal;
out vec4 out_Color;

uniform vec3 planet_color;
uniform vec3 light_color;
uniform vec1 light_intensity;
uniform vec3 light_position;


void main() {
	out_Color = vec4(planet_color, 1.0); simple shader of planets



}
