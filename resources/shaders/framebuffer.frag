#version 150
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D TextureFragment;

void main()
{
	FragColor = texture(TextureFragment, TexCoords);
}