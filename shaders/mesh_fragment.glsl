#version 330 core

precision mediump float;

out vec4 FragColor;

in vec3 diffuseColor; // objectColor
in vec2 texCoord;
in vec3 normal;
in vec3 FragPos; 

uniform vec4 ambient;
uniform vec3 viewPos; 
uniform vec3 lightPos; 
uniform vec4 lightColor;
uniform float shininess;

// texture samplers
uniform sampler2D texture1;
uniform bool useDiffuseMap;


void main()
{
	// ambient
    vec3 ambientLight = ambient.xyz * lightColor.xyz;
    
	// diffuse 
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor.xyz;

    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor.xyz; 

    // combine
    vec3 result = (ambientLight + diffuse + specular) * diffuseColor;

    // add texture
    if (useDiffuseMap) {
        vec3 textureColor = texture(texture1, texCoord).rgb;
        result = mix(result, textureColor, 0.5);
    }        
 
    FragColor = vec4(result, 1.0);

}