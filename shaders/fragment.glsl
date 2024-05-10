#version 330 core

precision mediump float;

out vec4 FragColor;

in vec3 objectColor;
in vec2 texCoord;

uniform vec4 ambient;

// texture samplers
uniform sampler2D texture1;


void main()
{
	
	vec4 diffuse = vec4(objectColor, 1.0f) * ambient;
	FragColor = mix(texture(texture1, texCoord), diffuse, 0.5);
	
	
	
	//vec2 fakeTexCoord = gl_FragCoord.xy / vec2(800.0, 600.0); // Replace with your actual screen size
   // FragColor = texture(texture1, fakeTexCoord);
}
