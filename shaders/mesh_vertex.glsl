#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;

//out vec3 diffuseColor;
out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace[9];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Combination of lights view and projection matrices.
uniform mat4 lightSpaceMatrix[9];

void main()
{
	// Vertex position from object space to world space.
	FragPos = vec3(model * vec4(aPos, 1.0));
	
	// Normal to world space.
	Normal = mat3(transpose(inverse(model))) * aNormal;

	// Passes through the texture coordinates.
	TexCoords = vec2(aTexCoord.x, aTexCoord.y);

	// Position of vertex world space into light clip space.
	// The result FragPosLightSpace[i] is the position of
	// the vertex as seen from the lights point of view.
	for(int i = 0; i < 9; i++){
		FragPosLightSpace[i] = lightSpaceMatrix[i] * vec4(FragPos, 1.0);
	} 
	
	// World space position to clip space.
  gl_Position = projection * view * vec4(FragPos, 1.0);
}

