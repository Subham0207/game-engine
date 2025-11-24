#version 420 core

layout (location = 0) in vec3 apos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec4 aColor;
layout (location = 4) in vec3 aTangent;
layout (location = 5) in vec3 aBitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightProjection;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out vec4 Color;
out vec3 Tangent;
out vec3 Bitangent;
out vec4 FragPosLightSpace;

void main()
{
    vec4 totalPosition = vec4(apos, 1.0); // No skinning, just use position
    vec3 totalNormal = aNormal;          // No skinning, use original normal

    mat4 viewModel = view * model;
    gl_Position = projection * viewModel * totalPosition;

    FragPos = vec3(model * totalPosition);
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    Normal = normalize(normalMatrix * totalNormal);
    Tangent   = normalize(normalMatrix * aTangent);
    Bitangent = normalize(normalMatrix * aBitangent);
    FragPosLightSpace = lightProjection * model * totalPosition;


    TexCoords = aTexCoords;
    Color = aColor;
}