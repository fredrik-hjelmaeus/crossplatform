#version 330 core
precision mediump float;

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;

void main()
{
    FragColor = vec4(1.0f);//texture(screenTexture, TexCoords);
}