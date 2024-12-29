#version 420 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_Texture;

void main() {
    float alpha = texture(u_Texture, TexCoords).r;
    if (alpha < 0.1) discard;  // Remove background
    FragColor = vec4(1.0, 1.0, 1.0, alpha);  // White text
}
