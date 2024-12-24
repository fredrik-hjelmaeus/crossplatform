#version 330 core

precision mediump float;

out vec4 FragColor;

//in vec3 diffuseColor; // diffuse base color from attribute data  (NOT USED ATM)
in vec2 TexCoords; 

struct Material {
    sampler2D diffuse; 
    sampler2D specular; 
    vec4 diffuseColor;
    float shininess;
    float diffuseMapOpacity;
};
uniform Material material;

void main()
{
    vec3 diffuseTexture = texture(material.diffuse, TexCoords).rgb;
    vec3 diffuse = mix(material.diffuseColor.xyz, diffuseTexture, material.diffuseMapOpacity);
        
    vec3 result = diffuse;
    FragColor = vec4(result, 1.0);
}
