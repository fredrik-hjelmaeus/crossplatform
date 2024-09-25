#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;

out vec3 diffuseColor;
out vec2 texCoord;
out vec3 normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	diffuseColor = aColor;
	normal = mat3(transpose(inverse(model))) * aNormal;
	texCoord = vec2(aTexCoord.x, aTexCoord.y);
	
	FragPos = vec3(model * vec4(aPos, 1.0));
	gl_Position = projection * view * vec4(FragPos, 1.0);
}

