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

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
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
    bool hasDiffuseMap;
    float shininess;
    float diffuseMapOpacity;
    vec4 diffuseColor;
};

#define NR_POINT_LIGHTS 4
#define NR_SPOT_LIGHTS 4

uniform Material material;
uniform DirLight dirLight;
uniform SpotLight spotLights[4];
uniform PointLight pointLights[4];
uniform vec3 viewPos;
uniform bool blinn;
uniform bool gamma;

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{   
    // properties
    vec3 norm =    normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // == =====================================================
    // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
    // For each phase, a calculate function is defined that calculates the corresponding color
    // per lamp. In the main() function we take all the calculated colors and sum them up for
    // this fragment's final color.
    // == =====================================================

    // phase 1: directional lighting
      vec3 result = CalcDirLight(dirLight, norm, viewDir);

    // phase 2: point lights
     for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);      
    
    // phase 3: spot lights
    for(int i = 0; i < NR_SPOT_LIGHTS; i++)
        result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);        
      
    
    if(gamma)
        result = pow(result, vec3(1.0/2.2));
    FragColor = vec4(result, 1.0f);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{   
    //light.position = vec3(1.0, 3.0, 1.0);
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;//pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    if(blinn)
    {
        // Blinn
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    }
    else
    {
        // Phong
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    }
    // attenuation
    float distance = length(light.position - fragPos);
    float gammaDistanceAttenuation = gamma ? distance * distance : distance;
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * gammaDistanceAttenuation);    
    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = vec3(0.0);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);
    if(material.hasDiffuseMap == true){
        ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
        specular =light.specular * spec * vec3(texture(material.specular, TexCoords));
    }else {
        ambient = light.ambient * material.diffuseColor.rgb;
        diffuse = light.diffuse * diff * material.diffuseColor.rgb;
        specular = light.specular * spec * material.diffuseColor.rgb;
    }  
   
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
 
    return (ambient + diffuse + specular);
}

   

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    if(blinn)
    {
        // Blinn
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    }
    else
    {
        // Phong
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    }
    // combine results
    vec3 ambient = vec3(1.0);
    vec3 diffuse = vec3(1.0);
    vec3 specular = vec3(1.0);

    // TODO: avoid branching & bool for specular map aswell?. Atm we assume spec map if diffuse map is present.
     if(material.hasDiffuseMap == true){ 
         ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
         diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
         specular =light.specular * spec * vec3(texture(material.specular, TexCoords)); 
      }else { 
         ambient = light.ambient * material.diffuseColor.rgb;
         diffuse = light.diffuse * diff * material.diffuseColor.rgb;
         specular = light.specular * spec * material.diffuseColor.rgb;
    }
    return (ambient + diffuse + specular);
} 

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    if(blinn)
    {
        // Blinn
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    }
    else
    {
        // Phong
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    }
    // attenuation
    float distance = length(light.position - fragPos);
    float gammaDistanceAttenuation = gamma ? distance * distance : distance;
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * gammaDistanceAttenuation);    
    // combine results
    vec3 ambient = vec3(0.0);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);
    if(material.hasDiffuseMap == true){
        ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
        specular =light.specular * spec * vec3(texture(material.specular, TexCoords)); 
   }else {
        ambient = light.ambient * material.diffuseColor.rgb;
        diffuse = light.diffuse * diff * material.diffuseColor.rgb;
        specular = light.specular * spec * material.diffuseColor.rgb;
    }  
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

