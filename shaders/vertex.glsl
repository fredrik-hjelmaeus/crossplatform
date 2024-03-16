#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 texCoord;

void main()
{
	gl_Position = vec4(aPos, 1.0);
	ourColor = aColor;
	texCoord = vec2(aTexCoord.x, aTexCoord.y);
}


/* 
#version 300 es
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec2 a_texCoord;
out vec2 v_texCoord;
void main()
{
 gl_Position = a_position;
 v_texCoord = a_texCoord;
} */