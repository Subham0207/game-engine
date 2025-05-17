#version 420 core

layout (location = 0) in vec3 apos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec4 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out vec4 Color;

void main()
{
    vec4 totalPosition = vec4(apos, 1.0); // No skinning, just use position
    vec3 totalNormal = aNormal;          // No skinning, use original normal

    mat4 viewModel = view * model;
    gl_Position = projection * viewModel * totalPosition;

    FragPos = vec3(model * totalPosition);
    Normal = normalize(mat3(transpose(inverse(model))) * totalNormal);

    TexCoords = aTexCoords;
    Color = aColor;
}