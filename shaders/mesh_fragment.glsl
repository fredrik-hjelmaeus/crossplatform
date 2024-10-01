#version 330 core

precision mediump float;

out vec4 FragColor;

//in vec3 diffuseColor; // diffuse base color from attribute data  (NOT USED ATM)
in vec2 TexCoords; 
in vec3 Normal; 
in vec3 FragPos; 

struct Light {
    vec3 position; 
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec3 direction;
};

struct Material {
    sampler2D diffuse; // diffuseMap
    sampler2D specular; // specularMap
    float shininess;
    float diffuseMapOpacity;
    vec4 diffuseColor;
};

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main()
{
    // ambient
    vec3 ambient = light.ambient.xyz * texture(material.diffuse, TexCoords).rgb;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    //vec3 lightDir = normalize(light.position - FragPos);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 textureColor = texture(material.diffuse, TexCoords).rgb;
    vec3 blendedDiffuse = mix(material.diffuseColor.xyz, textureColor, material.diffuseMapOpacity);
    vec3 diffuse = light.diffuse.xyz * diff * blendedDiffuse;
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular.xyz * spec * texture(material.specular, TexCoords).rgb;  
        
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
    
}
