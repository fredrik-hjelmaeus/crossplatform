#version 330 core

precision mediump float;

out vec4 FragColor;

//in vec3 diffuseColor; // diffuse base color from attribute data  (NOT USED ATM)
in vec2 TexCoords; 
in vec3 Normal; 
in vec3 FragPos; 

struct Light {
    vec3 position; 
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 direction;
    
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
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
    

    // inside the spotlight cone
   
        // ambient
        vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

        // diffuse 
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(light.position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        
        // diffuse map + diffuse attribute color
        vec3 textureColor = texture(material.diffuse, TexCoords).rgb;
        vec3 blendedDiffuse = mix(material.diffuseColor.xyz, textureColor, material.diffuseMapOpacity);
        vec3 diffuse = light.diffuse * diff * blendedDiffuse;

        // specular
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); // 0.5 was 0.0 in default code.
        vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;  

        // check if the lighting is inside the spotlight cone
        float theta = dot(lightDir, normalize(-light.direction)); 

        // light falloff (soft edges)
        float epsilon = (light.cutOff - light.outerCutOff);
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        diffuse  *= intensity;
        specular *= intensity; 

        // attenuation
        float distance = length(light.position - FragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        
        ambient  *= attenuation; 
        diffuse  *= attenuation;
        specular *= attenuation; 

        vec3 result = ambient + diffuse + specular;
        FragColor = vec4(result, 1.0);
 
  
}
