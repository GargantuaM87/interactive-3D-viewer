#version 330 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

struct Material {
    sampler2D texture_diffuse1; //0
};

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    vec3 color;

    float constant;
    float linear;
    float quadratic;
};
uniform DirectionalLight dLight;
uniform PointLight pLight;
uniform Material u_mat;
uniform vec3 u_viewPos;
uniform sampler2D depthMap; //1
uniform samplerCube depthCubeMap; //2
uniform float far_plane;

vec3 color;

vec3 sampleOffsetDirections[20] = vec3[](
        vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
        vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
        vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
        vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
        vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
    );

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir);
float ShadowCalculation(vec4 fragPosLightSpace, float bias);
float PointShadowCalculation(vec3 fragPos);

void main()
{
    vec3 normal = fs_in.Normal;
    vec3 fragPos = fs_in.FragPos;
    //color = texture(u_mat.texture_diffuse1, fs_in.TexCoords).rgb;
    color = vec3(0.4, 0.0, 0.4);

    vec3 viewDir = normalize(u_viewPos - fragPos);

    vec3 result = CalcDirLight(dLight, normal, viewDir);
    result *= CalcPointLight(pLight, normal, fragPos, viewDir);
    FragColor = vec4(result, 1.0);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // diffuse
    float diff = max(dot(lightDir, normal), 0.0);
    // specular
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    // combine
    vec3 ambient = light.ambient * color;
    vec3 diffuse = light.diffuse * diff * color;
    vec3 specular = light.specular * spec;

    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    float shadow = ShadowCalculation(fs_in.FragPosLightSpace, bias);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

    return lighting;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    // specular
    float spec = pow(max(dot(viewDir, halfwayDir), 0.0), 32.0);

    vec3 ambient = 0.3 * color;
    vec3 diffuse = diff * color;
    vec3 specular = light.color * spec;

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1 / (light.constant + (light.linear * distance) + (light.quadratic * distance * distance));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    // calculating shadow
    float shadow = PointShadowCalculation(fragPos);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

    return lighting;
}

float ShadowCalculation(vec4 fragPosLightSpace, float bias) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // transform to range [0, 1] to use to sample from depth map
    // closest depth from light's POV
    float closestDepth = texture(depthMap, projCoords.xy).r;
    float currentDepth = projCoords.z; // the depth of the current fragment the fragment shader is running on
    // depth comparison
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(depthMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    if (projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

float PointShadowCalculation(vec3 fragPos) {
    vec3 fragToLight = fragPos - pLight.position; // from the lightPos to fragPos
    // normalized depth value between light source and its closest visible fragment
    float closestDepth = texture(depthCubeMap, fragToLight).r;
    closestDepth *= far_plane; // transform range from [0,1] to [0, far_plane];
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(u_viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    for (int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthCubeMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= far_plane; // undo mapping [0;1]
        if (currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);

    return shadow;
}
