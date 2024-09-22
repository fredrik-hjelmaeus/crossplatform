#version 330 core

precision mediump float;

out vec4 FragColor;

in vec3 diffuseColor;
in vec2 texCoord;
in vec3 normal;
in vec3 FragPos; 

uniform vec4 ambient;
uniform vec3 lightPos; 
//uniform vec3 lightColor;

// texture samplers
uniform sampler2D texture1;

uniform bool useDiffuseMap;

void main()
{
	// New------------------
	// Temp:
	vec3 lightColor = vec3(0.5, 0.5, 1.0);
	// ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    

	// diffuse 
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
   
            
   // vec3 result = (ambient + diffuse) * diffuseColor;
    vec3 result = (ambient + diffuse) * vec3(1.0, 1.0, 1.0);
    FragColor = vec4(result, 1.0);

	// Before -------------------
	//vec4 diffuse = vec4(diffuseColor, 1.0f) * ambient;
	//if (useDiffuseMap) {
	//	FragColor = mix(texture(texture1, texCoord), diffuse, 0.5);
   // } else {
//		FragColor = diffuse;
//    }

}