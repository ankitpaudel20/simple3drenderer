#version 440 core

in vec3 TexCoords;

uniform samplerCube skybox; // cubemap texture sampler

out vec4 FragColor;


void main()
{
    FragColor =texture(skybox, TexCoords);
}