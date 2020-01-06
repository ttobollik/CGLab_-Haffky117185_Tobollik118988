#version 150

in  vec3 pass_Normal;
out vec4 out_Color;
uniform vec3 planet_color;

void main() {
  out_Color = vec4(planet_color, 1.0);
}
