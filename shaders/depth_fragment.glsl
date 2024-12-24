#version 330 core

void main()
{
    // We are not going to do anything in the fragment shader
    // We'll just output the depth value
    // The fragment's depth value is already set by the vertex shader
    // We could also output a constant value if we wanted to
    // gl_FragDepth = 0.0;
}