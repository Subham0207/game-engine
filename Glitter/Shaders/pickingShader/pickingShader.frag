#version 330 core
out vec4 FragColor;

uniform vec4 pickColor; // Unique color for object picking

void main() {
    FragColor = pickColor; // Output the unique color directly
}
