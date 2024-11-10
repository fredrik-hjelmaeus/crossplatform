#version 330 core

precision mediump float;

out vec4 FragColor;

uniform vec4 lineColor;

void main()
{
    FragColor = vec4(lineColor);
}