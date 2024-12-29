#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

uniform mat4 u_MVP;

out vec2 TexCoords;

void main() {
    gl_Position = u_MVP * vec4(aPos, 1.0);
    TexCoords = aTexCoords;
}
