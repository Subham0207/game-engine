#version 330

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D gSampler;

void main()
{
    vec3 norm = normalize(Normal);
    FragColor = texture2D(gSampler, TexCoords);
}