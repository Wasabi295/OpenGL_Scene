#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexCoords;
out vec4 fragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform mat4 lightSpaceMatrix;

// WIND - ADAUGAT!
uniform float time;
uniform float windStrength;

void main()
{
    vec3 pos = vPosition;
    
    
    // Apply wind effect only to vertices with positive Y (top of vegetation)
    // This creates a swaying effect for trees and flowers
    if (pos.y > 0.5) {
        float windEffect = windStrength * pos.y; // Stronger at the top
        
        // Create wave pattern using sine waves
        float wind = sin(time * 2.0 + pos.x * 0.5 + pos.z * 0.5) * windEffect;
        wind += sin(time * 1.5 + pos.x * 0.3) * windEffect * 0.5;
        
        // Apply wind displacement on X and Z axes
        pos.x += wind;
        pos.z += wind * 0.7;
    }
    
    
    vec4 worldPos = model * vec4(pos, 1.0);
    vec4 viewPos  = view * worldPos;

    fPosition = viewPos.xyz;
    fNormal   = normalize(normalMatrix * vNormal);
    fTexCoords = vTexCoords;

    fragPosLightSpace = lightSpaceMatrix * worldPos;

    gl_Position = projection * viewPos;
}