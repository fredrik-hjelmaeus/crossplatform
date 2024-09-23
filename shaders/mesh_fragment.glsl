#version 330 core

precision mediump float;

out vec4 FragColor;

in vec3 diffuseColor;
in vec2 texCoord;
in vec3 normal;
in vec3 FragPos; 

uniform vec4 ambient;
uniform vec3 lightPos; 
uniform vec4 lightColor;

// texture samplers
uniform sampler2D texture1;

uniform bool useDiffuseMap;

void main()
{
	// ambient
    vec4 ambientLight = ambient * lightColor;
    
	// diffuse 
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuseShading = diff * lightColor.xyz;

    // Combine diffuse shading with object diffuse color
    vec3 diffuseComponent = diffuseShading * diffuseColor;
   
    if (useDiffuseMap) {
        vec3 textureColor = texture(texture1, texCoord).rgb;
        diffuseComponent = mix(diffuseComponent, textureColor, 0.5);
    }         
 
    // Combine ambient and diffuse components
    vec3 result = ambientLight.xyz + diffuseComponent;

    FragColor = vec4(result, 1.0);

}