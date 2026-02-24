#version 420 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture; // The raw HDR scene
uniform float exposure = 1.0;

vec3 ACESFilmToneMapping(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

vec3 ReinhardToneMapping(vec3 hdrColor, float exposure)
{
    return vec3(1.0) - exp(-hdrColor * exposure);
}

void main() {
    vec3 hdrColor = texture(screenTexture, TexCoords).rgb;

    // --- 1. Simple Bloom Threshold ---
    // Extract brightness; if it's over 1.0, it's "glowing"
    float brightness = dot(hdrColor, vec3(0.2126, 0.7152, 0.0722));
    vec3 bloomResult = (brightness > 1.0) ? hdrColor : vec3(0.0);

    // --- 2. Tonemapping (Reinhard) ---
    // This prevents "clamping" where bright areas just look like flat white
    vec3 mapped = ACESFilmToneMapping(hdrColor * exposure);

    // --- 3. Gamma Correction ---
    // Standard monitors use a gamma of 2.2
    mapped = pow(mapped, vec3(1.0 / 2.2));

    FragColor = vec4(mapped, 1.0);
}