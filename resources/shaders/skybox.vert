#version 150
#extension GL_ARB_explicit_attrib_location : require
layout(location = 0) in vec3 in_Position; 

out vec3 pass_PixelPosition;

uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;

void main() {
	pass_PixelPosition = in_Position;
	gl_Position = (ProjectionMatrix  * ViewMatrix) * vec4(in_Position, 1.0);
}

