#version 410 core

in vec3 fPosition;            // view-space
in vec3 fNormal;              // view-space
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

// Textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

// Lights in VIEW SPACE
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 pointLightPos;
uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform int spotLightEnabled;

// FOG - ADAUGAT!
uniform int fogEnabled;
uniform float fogDensity;

// Material
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform Material material;

float shadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 dirLightVecFromFragToLight)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z > 1.0)
        return 0.0;

    float ndotl = max(dot(normal, dirLightVecFromFragToLight), 0.0);
    float bias = max(0.0035 * (1.0 - ndotl), 0.0008);

    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    float shadow = 0.0;

    for (int x = -2; x <= 2; x++) {
        for (int y = -2; y <= 2; y++) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (projCoords.z - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }
    shadow /= 25.0;

    return shadow;
}

vec3 computeDirLight(vec3 N, vec3 V)
{
    vec3 L = normalize(-lightDir);
    float diff = max(dot(N, L), 0.0);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), material.shininess);

    vec3 ambient  = 0.18 * material.ambient * lightColor;
    vec3 diffuse  = diff * material.diffuse * lightColor;
    vec3 specular = 0.45 * spec * material.specular * lightColor;

    return ambient + diffuse + specular;
}

vec3 computePointLight(vec3 N, vec3 V, vec3 fragPos)
{
    vec3 L = normalize(pointLightPos - fragPos);
    float diff = max(dot(N, L), 0.0);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), material.shininess);

    float distance = length(pointLightPos - fragPos);
    float attenuation = 1.0 / (1.0 + 0.045 * distance + 0.0075 * distance * distance);

    vec3 pointColor = vec3(1.0, 0.80, 0.55);

    vec3 ambient  = 0.10 * material.ambient * pointColor * attenuation;
    vec3 diffuse  = 1.60 * diff * material.diffuse * pointColor * attenuation;
    vec3 specular = 0.60 * spec * material.specular * pointColor * attenuation;

    return ambient + diffuse + specular;
}

vec3 computeSpotLight(vec3 N, vec3 V, vec3 fragPos)
{
    if (spotLightEnabled == 0) return vec3(0.0);

    vec3 L = normalize(spotLightPos - fragPos);

    float theta = dot(L, normalize(-spotLightDir));
    float innerCut = cos(radians(15.0));  // Wider cone
    float outerCut = cos(radians(25.0));  // Softer edge
    float intensity = clamp((theta - outerCut) / (innerCut - outerCut), 0.0, 1.0);

    float diff = max(dot(N, L), 0.0);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), material.shininess);

    float distance = length(spotLightPos - fragPos);
    float attenuation = 1.0 / (1.0 + 0.009 * distance + 0.0032 * distance * distance);  // Reduced attenuation

    vec3 spotColor = vec3(1.0, 0.95, 0.85);  // Slightly warm flashlight color

    vec3 ambient  = 0.15 * material.ambient * spotColor * attenuation * intensity;
    vec3 diffuse  = 2.5 * diff * material.diffuse * spotColor * attenuation * intensity;  // Much stronger
    vec3 specular = 1.0 * spec * material.specular * spotColor * attenuation * intensity;

    return ambient + diffuse + specular;
}

void main()
{
    vec4 texColor = texture(diffuseTexture, fTexCoords);

    if (texColor.a < 0.30)
        discard;

    vec3 N = normalize(fNormal);
    vec3 V = normalize(-fPosition);

    vec3 dirFromFragToLight = normalize(-lightDir);

    float shadow = shadowCalculation(fragPosLightSpace, N, dirFromFragToLight);

    vec3 lighting =
        computeDirLight(N, V) +
        computePointLight(N, V, fPosition) +
        computeSpotLight(N, V, fPosition);

    vec3 dirOnly = computeDirLight(N, V);
    vec3 finalLight = (dirOnly * (1.0 - shadow)) + (lighting - dirOnly);

    vec3 color = finalLight * texColor.rgb;


    if (fogEnabled == 1) {
        // Exponential fog based on distance from camera
        float fogDistance = length(fPosition);
        float fogFactor = exp(-fogDensity * fogDistance);
        fogFactor = clamp(fogFactor, 0.0, 1.0);
        
        // Grey fog color that matches your scene
        vec3 fogColor = vec3(0.5, 0.5, 0.55);
        
        // Mix scene color with fog
        color = mix(fogColor, color, fogFactor);
    }
   

    fColor = vec4(color, texColor.a);
}