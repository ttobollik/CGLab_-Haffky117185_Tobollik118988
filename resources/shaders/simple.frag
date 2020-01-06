#version 150

in vec3 pass_Normal;
in vec3 pass_VertexPosition;
in vec3 pass_CameraPosition;
in mat4 pass_ViewMatrix;

out vec4 out_Color;

uniform vec3 planet_color;
uniform vec3 light_color;
uniform float light_intensity;
uniform vec3 light_position;

vec3 ambient_color = vec3(0.2f, 0.2f, 0.2f);
vec3 diffuse_color = vec3(0.3f, 0.3f, 0.3f);
vec3 specular_color = vec3(1.0f, 1.0f, 1.0f);
float alpha = 0;



void main() {
	//out_Color = vec4(planet_color, 1.0); //--> simple shader to just draw color



	//Variables from function
	//some geometric functions already exists in glsl (length, normalize ... ) p. 86 -89
	vec3 normal = normalize(pass_Normal);
	//Light Direction vector is the Light Position minus the Vertex Position
	vec3 light_vector = normalize(light_position - pass_VertexPosition);
	vec3 view_vector = normalize(pass_CameraPosition - pass_VertexPosition);

	vec3 halfway_vector = normalize(light_vector + view_vector);

	//Formel für Blinn Phong: Ambient + LichtIntensitaetsformel * DiffuseLicht Formel * refelctivity + SpecularLight(Blinn)
	//Ambient und Intenstitaet gegeben

	float diffuse_light = max(dot(normal, light_vector), 0) * light_intensity;
	vec3 diffuse = diffuse_color * light_color * diffuse_light;

	//This is the Blinn Phong Formula
	//vec3 specular = (halfway_vector * view_vector);
	float specular_light = pow(max(dot(halfway_vector, normal), 0),4)*light_intensity;
	vec3 specular = specular_color * specular_light;

	out_Color = vec4((ambient_color + diffuse)* planet_color + specular *light_color, 1.0);


}
