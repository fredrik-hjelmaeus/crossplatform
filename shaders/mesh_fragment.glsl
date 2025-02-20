#version 330 core

precision mediump float;

out vec4 FragColor;

//in vec3 diffuseColor; // diffuse base color from attribute data  (NOT USED ATM)
in vec2 TexCoords; 
in vec3 Normal; 
in vec3 FragPos; 
in vec4 FragPosLightSpace[9];

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
uniform sampler2D shadowMap;

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir,int lightIndex);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir,int lightIndex);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir,int lightIndex);
float ShadowCalculation(vec4 fragPosLightSpace);
float calcShadow(vec4 FragPosLightSpace,vec3 normal, vec3 lightDir, sampler2D shadowMap);

void main()
{   
   
    // properties
    vec3 norm    = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // == =====================================================
    // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
    // For each phase, a calculate function is defined that calculates the corresponding color
    // per lamp. In the main() function we take all the calculated colors and sum them up for
    // this fragment's final color.
    // ========================================================
   // vec3 result = vec3(0.0);
    int lightIndex = 0;
    // phase 1: directional lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir,lightIndex);
    lightIndex++;

    // phase 2: point lights
      for(int i = 0; i < NR_POINT_LIGHTS; i++){
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir,lightIndex);      
        lightIndex++;
      }
    
    // phase 3: spot lights
     for(int i = 0; i < NR_SPOT_LIGHTS; i++){
        result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir,lightIndex);      
        lightIndex++;
     }
   
    if(gamma)
        result = pow(result, vec3(1.0/2.2)); 
    FragColor = vec4(result, 1.0); 
   
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir,int lightIndex)
{   
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

    // shadow
    float shadow = calcShadow(FragPosLightSpace[lightIndex], normal, lightDir, shadowMap);

    return (ambient + (1.0 - shadow) * (diffuse + specular));
}

   

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir,int lightIndex)
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
    vec3 color = vec3(texture(material.diffuse, TexCoords));

    // TODO: avoid branching & bool for specular map aswell?. Atm we assume spec map if diffuse map is present.
    if(material.hasDiffuseMap == true){ 
         ambient = light.ambient * color;
         diffuse = light.diffuse * diff * color;
         specular =light.specular * spec * vec3(texture(material.specular, TexCoords)); 
    }else { 
         ambient = light.ambient * material.diffuseColor.rgb;
         diffuse = light.diffuse * diff * material.diffuseColor.rgb;
         specular = light.specular * spec * material.diffuseColor.rgb;
    }
  
    float shadow = calcShadow(FragPosLightSpace[lightIndex], normal, lightDir, shadowMap);

    return (ambient + (1.0 - shadow) * (diffuse + specular));
} 

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir,int lightIndex)
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

    // shadow
    float shadow = calcShadow(FragPosLightSpace[lightIndex], normal, lightDir, shadowMap);

    return (ambient + (1.0 - shadow) * (diffuse + specular));
}

/* float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}  */

float calcShadow(vec4 FragPosLightSpace,vec3 normal, vec3 lightDir, sampler2D shadowMap){
     // perform perspective divide
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
   
    float shadow = 0.0;
    //float bias = 0.005;
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += projCoords.z - bias > pcfDepth  ? 1.0 : 0.0;        
        }
    }
    shadow /= 9.0;

    // If beyond far_plane do not shadow
    if(projCoords.z > 1.0)
	{
		shadow = 1.0;
	} 
    return shadow;
}