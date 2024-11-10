#version 330 core

precision mediump float;

out vec4 FragColor;

// See OpenGL_ES_3.0_Programming_Guide page 205/206 when you need to make this a textured sprite/point

uniform vec4 pointColor;

void main()
{
    FragColor = vec4(pointColor);
}