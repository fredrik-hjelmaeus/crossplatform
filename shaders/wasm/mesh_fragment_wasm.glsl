#version 300 es

precision mediump float;

out vec4 FragColor;

in vec3 objectColor;
in vec2 texCoord;

uniform vec4 ambient;

// texture samplers
uniform sampler2D texture1;

uniform bool useDiffuseMap;

void main()
{
	
	vec4 diffuse = vec4(objectColor, 1.0f) * ambient;
	if (useDiffuseMap) {
		FragColor = mix(texture(texture1, texCoord), diffuse, 0.5);
    } else {
		FragColor = diffuse;
    }
	
	
	
	//vec2 fakeTexCoord = gl_FragCoord.xy / vec2(800.0, 600.0); // Replace with your actual screen size
   // FragColor = texture(texture1, fakeTexCoord);
}
