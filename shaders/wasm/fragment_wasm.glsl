#version 300 es

precision mediump float;

out vec4 FragColor;
uniform vec4 color;

in vec3 ourColor;
in vec2 TexCoord;

// texture samplers
uniform sampler2D texture1;
//uniform sampler2D texture2;

void main() {
//  FragColor = vec4(ourColor,1.0f);
FragColor = mix(texture(texture1, TexCoord), vec4(ourColor,1.0f), 0.5);
  // linearly interpolate between both textures (80% container, 20% awesomeface)
//	FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
}