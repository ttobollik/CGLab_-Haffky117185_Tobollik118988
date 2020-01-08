#version 150
#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;


//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;

out vec3 pass_Normal;
out vec3 pass_VertexPosition;
out vec3 pass_CameraPosition;
out mat4 pass_ViewMatrix;

/*vertex shader processes one vertex at a time and computes vertex position in clip space*/
/*must have as input vertex position in object space and all other certex attributes*/

void main(void)
{
	gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);

	//is already interpolated (wiki)
	pass_Normal = (NormalMatrix * vec4(in_Normal, 0.0)).xyz;

	/*component selection syntax allows multiple components to be selected by appending their names after the period p.53*/
	pass_VertexPosition = (ModelMatrix * vec4(in_Position, 1.0)).xyz;
	pass_CameraPosition = (ViewMatrix * vec4(0.0, 0.0, 0.0, 1.0)).xyz;


}
