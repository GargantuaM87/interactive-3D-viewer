#version 330 core

out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

uniform vec3 camPos;
uniform vec3 lightPositions[2];
uniform vec3 lightColors[2];

uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 baseReflectivity);
float DistributionGGX(vec3 normal, vec3 halfway, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 normal, vec3 viewDir, vec3 lightDir, float roughness);

void main()
{
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(camPos - WorldPos);
    vec3 irradiance = vec3(0.0); // L0, measures the reflected sum of the lights irradiance
    for (int i = 0; i < 2; ++i) { // substitute to solving an integral
        vec3 lightDir = normalize(lightPositions[i] - WorldPos);
        vec3 halfway = normalize(viewDir + lightDir); // halfway vector

        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance); // attenuate by more physically correct inverse-square law
        vec3 radiance = lightColors[i] * attenuation;

        //a simplification that most dieletrics have a base reflectivity of 0.04
        vec3 baseReflectivity = vec3(0.04); //a surface's response at normal incidence
        baseReflectivity = mix(baseReflectivity, albedo, metallic); // basically lerp

        vec3 F = fresnelSchlick(clamp(dot(normal, viewDir), 0.0, 1.0), baseReflectivity); // fresnel equation
        float NDF = DistributionGGX(normal, halfway, roughness); // normal distribution function
        float G = GeometrySmith(normal, viewDir, lightDir, roughness); // geometry function
        // cook torrange
        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
        vec3 specular = numerator / denominator; // specular lighting is equal to the cook-torrance equation

        vec3 kS = F; // light that gets reflected, namely the fresnel equation
        vec3 kD = vec3(1.0) - kS; // whatever gets reflected is refracted
        kD *= 1.0 - metallic; // this accounts for conductors (metals) as they only have specular lighting and no diffusion

        float NdotL = max(dot(normal, lightDir), 0.0);
        irradiance += (kD * albedo / PI + specular) * radiance * NdotL; // full reflectance equation
    }
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + irradiance;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}

vec3 fresnelSchlick(float cosTheta, vec3 baseReflectivity)
{
    return baseReflectivity + (1.0 - baseReflectivity) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 normal, vec3 halfway, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(normal, halfway), 0.0);
    float NdotH2 = NdotH * NdotH;

    float numerator = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return numerator / denom;
}

float GeometrySchlickGGX(float NDotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NDotV;
    float denom = NDotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 normal, vec3 viewDir, vec3 lightDir, float roughness)
{
    float NdotV = max(dot(normal, viewDir), 0.0);
    float NdotL = max(dot(normal, lightDir), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
