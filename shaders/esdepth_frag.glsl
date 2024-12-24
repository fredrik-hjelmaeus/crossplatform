#version 330 core
precision mediump float;

in vec4 v_color;    // input vertex color from vertex shader

out vec4 fragColor; // output fragment color
void main()
{
    fragColor = vec4(1.0);
}