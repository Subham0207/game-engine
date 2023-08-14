#version 330 core

layout (location = 0) in vec3 apos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

void main()
{
	gl_Position = projection * view * model * vec4(apos, 1.0);
	FragPos = vec3(model * vec4(apos, 1.0));
	Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
}