#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 6) in ivec4 boneIds;
layout (location = 7) in vec4 weights;

uniform mat4 model;
uniform mat4 lightSpaceMatrix; // spot light projection * view
uniform mat4 finalBonesMatrices[100];
uniform bool isAnimated;

void main()
{
    if(isAnimated)
    {
        vec4 totalPosition = vec4(0.0);
        for(int i = 0 ; i < 4 ; i++) {
                if(boneIds[i] == -1) continue;
                if(boneIds[i] >= 100) {
                    totalPosition = vec4(aPos, 1.0);
                    break;
                }
                vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos, 1.0);
                totalPosition += localPosition * weights[i];
        }

        gl_Position = lightSpaceMatrix * model * totalPosition;
    }
    else
    {
        gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
    }
}
