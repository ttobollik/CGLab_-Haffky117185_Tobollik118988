#version 150


//basically a simple shader that receives texture and passes color out according to texture, doesnt need enhancement
in vec3 pass_PixelPosition;
out vec4 out_Color;

uniform samplerCube skybox;

void main() {
	out_Color = texture(skybox, pass_PixelPosition);
}

