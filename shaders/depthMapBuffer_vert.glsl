#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;

in vec4 a_color; // input vertex color

//out vec3 diffuseColor;
out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
out vec4 v_color; // output vertex color

uniform mat4 lightSpaceMatrix;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	// Convert the vertex position to world coordinates
	FragPos = vec3(model * vec4(aPos, 1.0));
	v_color = a_color;
	//Normal = mat3(transpose(inverse(model))) * aNormal;
	//TexCoords = vec2(aTexCoord.x, aTexCoord.y);
	
	//gl_Position = projection * view * vec4(aPos, 1.0);
   // gl_Position = projection * view * vec4(FragPos, 1.0);
	//gl_Position = projection * view * model * vec4(aPos, 1.0);
	gl_Position = lightSpaceMatrix * projection * view * model * vec4(aPos, 1.0);
}

