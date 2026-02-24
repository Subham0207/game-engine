#version 420 core

out vec2 TexCoords;

void main() {
    // Generates a triangle that covers the -1 to 1 clip space
    uint id = gl_VertexID;
    float x = -1.0 + float((id & 1) << 2);
    float y = -1.0 + float((id & 2) << 1);

    TexCoords = vec2(x * 0.5 + 0.5, y * 0.5 + 0.5);
    gl_Position = vec4(x, y, 0.0, 1.0);
}