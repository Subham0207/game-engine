#version 420 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture; // The raw HDR scene
uniform float exposure = 1.0;

void main() {
    vec3 hdrColor = texture(screenTexture, TexCoords).rgb;

    // --- 1. Simple Bloom Threshold ---
    // Extract brightness; if it's over 1.0, it's "glowing"
    float brightness = dot(hdrColor, vec3(0.2126, 0.7152, 0.0722));
    vec3 bloomResult = (brightness > 1.0) ? hdrColor : vec3(0.0);

    // --- 2. Tonemapping (Reinhard) ---
    // This prevents "clamping" where bright areas just look like flat white
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);

    // --- 3. Gamma Correction ---
    // Standard monitors use a gamma of 2.2
    mapped = pow(mapped, vec3(1.0 / 2.2));

    FragColor = vec4(mapped, 1.0);
}