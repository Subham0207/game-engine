#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord0;

void main()
{
    gl_Position = projection * view * model * vec4(Position, 1.0);
    TexCoord0 = TexCoord;
}