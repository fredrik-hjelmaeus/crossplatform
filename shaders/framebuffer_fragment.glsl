#version 330 core

precision mediump float;

in vec2 TexCoord;
uniform sampler2D depthMap;

out vec4 FragColor;

void main() {
    //float depthValue = texture(depthMap, TexCoord).r;
    //FragColor = vec4(vec3(depthValue), 1.0);
    FragColor = vec4(1.0,1.0,1.0,1.0);
}