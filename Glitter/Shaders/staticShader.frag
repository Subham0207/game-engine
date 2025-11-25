#version 420 core

out vec4 FragColor;
in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;

layout(binding = 1) uniform sampler2D albedoMap;
layout(binding = 2) uniform sampler2D normalMap;
layout(binding = 3) uniform sampler2D metallicMap;
layout(binding = 4) uniform sampler2D roughnessMap;
layout(binding = 5) uniform sampler2D aoMap;

//IBL
layout(binding = 6) uniform samplerCube irradianceMap;
layout(binding = 7) uniform samplerCube prefilterMap;
layout(binding = 8) uniform sampler2D brdfLUT;

//Shadow
layout(binding = 9) uniform sampler2D shadowMap;
in vec4 FragPosLightSpace;

struct PointLight {
    vec3 position;
    //change name from diffuse to color
    vec3 diffuse;
    float intensity;
};

struct DirectionalLight {
    vec3 direction;  // direction *from* light *towards* the scene
    vec3 color;
    float intensity;
};

struct SpotLight {
    vec3 position;
    vec3 direction;   // direction the spotlight is pointing
    vec3 color;
    float intensity;
    float innerCutoff; // cos(innerAngle in radians)
    float outerCutoff; // cos(outerAngle in radians)
};

// uniform int numSpotLights;
uniform SpotLight spotLights[1];

uniform DirectionalLight dirLights[1];

// Only handling Point lights for now
uniform PointLight pointLights[4];

uniform vec3 viewPos;

const float PI = 3.14159265359;
  
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
vec3 getNormalFromMap();
vec3 EvaluatePBRLight(
    vec3 N,
    vec3 V,
    vec3 L,
    vec3 albedo,
    float metallic,
    float roughness,
    vec3 F0,
    vec3 radiance
);
float ShadowCalculation();

void main()
{
    vec4 albedoSample = texture(albedoMap, TexCoords);
    vec3 albedo = pow(albedoSample.rgb, vec3(2.2));
    float alpha = albedoSample.a;

    float metallic  = texture(metallicMap, TexCoords).r;
    float roughness = texture(roughnessMap, TexCoords).r;
    float ao        = texture(aoMap, TexCoords).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(viewPos - FragPos);
    vec3 R = reflect(-V, N); 

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	           
    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(pointLights[i].position - FragPos);
        vec3 H = normalize(V + L);
        float distance    = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = pointLights[i].diffuse *  pointLights[i].intensity * attenuation;
        
        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);        
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular     = numerator / denominator;  
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);                
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
    }

    for(int i = 0; i < 1; ++i)
    {
        vec3 L = normalize(dirLights[i].direction);

        vec3 radiance = dirLights[i].color * dirLights[i].intensity;

        Lo += EvaluatePBRLight(N, V, L, albedo, metallic, roughness, F0, radiance);   
    }

    for (int i = 0; i < 1; ++i)
    {
        // Direction from fragment TO light
        vec3 L = normalize(spotLights[i].position - FragPos);

        float distance    = length(spotLights[i].position - FragPos);
        float attenuation = 1.0 / (distance * distance + 0.01);

        // Cone angle factor
        // L is from fragment -> light, so -L is from light -> fragment
        float cosTheta = dot(normalize(-L), normalize(spotLights[i].direction));

        // Smooth step between inner and outer cutoff
        float epsilon    = spotLights[i].innerCutoff - spotLights[i].outerCutoff;
        float spotFactor = clamp((cosTheta - spotLights[i].outerCutoff) / epsilon, 0.0, 1.0);

        if (spotFactor > 0.0)
        {
            vec3 radiance = spotLights[i].color
                            * spotLights[i].intensity
                            * attenuation
                            * spotFactor;

            Lo += EvaluatePBRLight(N, V, L, albedo, metallic, roughness, F0, radiance);
        }
    }
  
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse      = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;

    float shadow = ShadowCalculation();

    vec3 color = ambient + (1.0 - shadow) * Lo;
	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, alpha);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    vec3 N = normalize(Normal);
    vec3 T = normalize(Tangent);
    vec3 B = normalize(Bitangent);

    // Orthonormalize to be safe
    T = normalize(T - dot(T, N) * N);
    B = normalize(cross(N, T));

    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangentNormal);
}

vec3 EvaluatePBRLight(
    vec3 N,
    vec3 V,
    vec3 L,
    vec3 albedo,
    float metallic,
    float roughness,
    vec3 F0,
    vec3 radiance
) {
    vec3 H = normalize(V + L);

    float NDF = DistributionGGX(N, H, roughness);        
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;  // no diffuse for metals

    vec3 numerator    = NDF * G * F;
    float denom       = 4.0 * max(dot(N, V), 0.0)
                          * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denom;

    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = kD * albedo / PI;

    return (diffuse + specular) * radiance * NdotL;
}

float ShadowCalculation()
{
    // Perspective divide: convert to NDC
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;

    // Transform range from [-1..1] to [0..1]
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0)
        return 0.0;

    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float bias = max(0.05 * (1.0 - dot(Normal, dirLights[0].direction)), 0.005);
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

    return shadow;
}