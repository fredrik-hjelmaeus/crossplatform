#version 330 core

precision mediump float;

out vec4 FragColor;

in vec3 ourColor;
in vec2 texCoord;

// texture samplers
uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	//FragColor = mix(texture(texture1, texCoord), texture(texture2, texCoord), 0.5);
	FragColor = mix(texture(texture1, texCoord), vec4(ourColor,1.0f), 0.5);
	//FragColor = vec4(ourColor,1.0f);
	//FragColor = texture(texture1, texCoord);
	
	//vec2 fakeTexCoord = gl_FragCoord.xy / vec2(800.0, 600.0); // Replace with your actual screen size
   // FragColor = texture(texture1, fakeTexCoord);
}

/* #version 300 es
precision mediump float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor; 
uniform sampler2D s_texture;
void main()
{
 outColor = texture( s_texture, v_texCoord );
} */

