#version 330 core

precision mediump float;

out vec4 FragColor;

//in vec3 diffuseColor; // diffuse base color from attribute data  (NOT USED ATM)
in vec2 TexCoords; 
in vec3 Normal; 
in vec3 FragPos; 

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

struct Material {
    sampler2D diffuse;   // diffuseMap
    sampler2D specular;  // specularMap
    float shininess;
    float diffuseMapOpacity;
    vec4 diffuseColor;
};

uniform Material material;
uniform SpotLight spotLight;
uniform vec3 viewPos;

void main()
{   
        // inside the spotlight cone
   
        // ambient
        vec3 ambient = spotLight.ambient * texture(material.diffuse, TexCoords).rgb;

        // diffuse 
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(spotLight.position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        
        // diffuse map + diffuse attribute color
        vec3 textureColor = texture(material.diffuse, TexCoords).rgb;
        vec3 blendedDiffuse = mix(material.diffuseColor.xyz, textureColor, material.diffuseMapOpacity);
        vec3 diffuse = spotLight.diffuse * diff * blendedDiffuse;

        // specular
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); // 0.5 was 0.0 in default code.
        vec3 specular = spotLight.specular * spec * texture(material.specular, TexCoords).rgb;  

        // check if the lighting is inside the spotlight cone
        float theta = dot(lightDir, normalize(-spotLight.direction)); 

        // light falloff (soft edges)
        float epsilon = (spotLight.cutOff - spotLight.outerCutOff);
        float intensity = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0);
        diffuse  *= intensity;
        specular *= intensity; 

        // attenuation
        float distance = length(spotLight.position - FragPos);
        float attenuation = 1.0 / (spotLight.constant + spotLight.linear * distance + spotLight.quadratic * (distance * distance));
        
        ambient  *= attenuation; 
        diffuse  *= attenuation;
        specular *= attenuation; 

        vec3 result = ambient + diffuse + specular;
        FragColor = vec4(result, 1.0);
}
