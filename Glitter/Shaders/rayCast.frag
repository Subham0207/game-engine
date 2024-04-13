#version 330 core

out vec4 fragColor;

uniform vec3 rayColor;

void main() {
    fragColor = vec4(rayColor, 1.0);
}
