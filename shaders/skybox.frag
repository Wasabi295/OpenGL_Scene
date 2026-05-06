#version 410 core

in vec3 TexCoords;
out vec4 fColor;

uniform samplerCube skybox;

void main()
{    
    fColor = texture(skybox, TexCoords);
}