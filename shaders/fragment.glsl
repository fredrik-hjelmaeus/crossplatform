#version 330 core

precision mediump float;

out vec4 FragColor;

in vec3 ourColor;
in vec2 texCoord;

// texture samplers
uniform sampler2D texture1;


void main()
{
	
	
	FragColor = mix(texture(texture1, texCoord), vec4(ourColor,1.0f), 0.5);
	
	
	
	//vec2 fakeTexCoord = gl_FragCoord.xy / vec2(800.0, 600.0); // Replace with your actual screen size
   // FragColor = texture(texture1, fakeTexCoord);
}
