#version 420 core

out vec4 FragColor;
in vec3 localPos;

layout(binding = 0) uniform sampler2D equirectangularMap;

// Maximum allowed radiance value sampled from the HDRI during cubemap capture.
// Exposed as a uniform for debugging/tuning; extreme values can poison mipmap generation.
uniform float u_HDRClampMax;

const vec2 invAtan = vec2(0.1591, 0.3183); // (1/2pi, 1/pi)
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    //Here uv is texture coordinate. We are using sphere to sample coordinates on the map.
    vec2 uv = SampleSphericalMap(normalize(localPos)); // make sure to normalize localPos
    vec3 color = texture(equirectangularMap, uv).rgb;

    // Defensive: some HDRIs contain extreme peaks and/or invalid values (NaN/Inf)
    // that can poison automatic mipmap generation and show up as black blocks/spots in mip>0.
    // Clamp to a high but finite range and ensure non-negative radiance.
    // NOTE: If u_HDRClampMax is left at 0, fall back to a safe default.
    float clampMax = (u_HDRClampMax > 0.0) ? u_HDRClampMax : 65000.0;
    color = max(color, vec3(0.0));
    color = clamp(color, vec3(0.0), vec3(clampMax));

    FragColor = vec4(color, 1.0);
}
