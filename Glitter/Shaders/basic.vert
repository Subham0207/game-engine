#version 330 core

layout (location = 0) in vec3 apos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec4 aColor;
layout(location = 6) in ivec4 aboneIds; 
layout(location = 7) in vec4 aWeights;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out vec4 Color;

void main()
{
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(aboneIds[i] == -1) 
            continue;
        if(aboneIds[i] >=MAX_BONES) 
        {
            totalPosition = vec4(apos,1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[aboneIds[i]] * vec4(apos,1.0f);
        totalPosition += localPosition * aWeights[i];
        vec3 localNormal = mat3(finalBonesMatrices[aboneIds[i]]) * aNormal;
    }
		
    mat4 viewModel = view * model;
    gl_Position =  projection * viewModel * totalPosition;
    TexCoords = aTexCoords;
    Normal = mat3(transpose(inverse(model))) * aNormal;
	Color = aColor;
}