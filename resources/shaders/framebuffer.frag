#version 150
out vec4 FragColor;
in vec2 TexCoords;

uniform bool greyscale;
uniform bool horizontalMirror;
uniform bool verticalMirror;
uniform bool blur;

float offset = 1.0/300.0;

uniform sampler2D TextureFragment;

void main()
{

	vec2 texture_coordinates = TexCoords;

	/*main idea of a kernel is to sample color values from other parts of the texture
	if we take a small sample of area surrounding the pixel (offset!) we have a kernel(matrix)
	*/
	vec2 offsets[9] = vec2[](
		vec2(-offset, offset),
		vec2(0.0f, offset),
		vec2(offset, offset),
		vec2(-offset, 0.0f),
		vec2(0.0f, 0.0f),
		vec2(offset, 0.0f),
		vec2(-offset, -offset),
		vec2(0.0f, -offset),
		vec2(offset, -offset)
		);

	//we divide by 16 because later on we will sum up all surrounding colors and this would be very bright
	float kernel[9] = float[](
		1.0/16, 2.0/16, 1.0/16,
		2.0/16, 4.0/16, 2.0/16,
		1.0/16, 2.0/16, 1.0/16
		);


	if(horizontalMirror) {
		texture_coordinates.y = 1.0 - texture_coordinates.y; 
	}

	if(verticalMirror) {
		texture_coordinates.x = 1.0 - texture_coordinates.x; 
	}


	FragColor = texture(TextureFragment, texture_coordinates);

	if(blur) {
		vec3 sampleTex[9];
		for(int i = 0; i < 9; ++i) {
			sampleTex[i] = vec3(texture(TextureFragment, texture_coordinates.st + offsets[i]));
		}
		vec3 col = vec3(0.0);
		for(int i = 0; i < 9; ++i) {
			col += sampleTex[i] * kernel[i];
		}
		FragColor = vec4(col, 1.0);

	}

	if(greyscale) {
		float average = (FragColor.r + FragColor.g + FragColor.b) / 3.0;
		FragColor = vec4(average, average, average, 1.0);
	}


}