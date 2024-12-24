#version 330 core

precision mediump float;

out vec4 FragColor;

in vec4 v_color; // input vertex color from vertex shader

//in vec3 diffuseColor; // diffuse base color from attribute data  (NOT USED ATM)
in vec2 TexCoords; 
in vec3 Normal; 
in vec3 FragPos; 

struct Material {
    sampler2D diffuse;   // diffuseMap
    sampler2D specular;  // specularMap
    bool hasDiffuseMap;
    float shininess;
    float diffuseMapOpacity;
    vec4 diffuseColor;
};



uniform Material material;




void main()
{   
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
