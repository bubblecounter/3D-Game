
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 atexCord;

out vec2 texCord;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
	Normal = aNormal;
	texCord = atexCord;
	FragPos = vec3(model * vec4(aPos, 1.0));// to pass the world space position to fragment shader
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
	gl_Position = projection * view *  vec4(FragPos, 1.0);
}