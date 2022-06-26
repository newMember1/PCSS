#version 330 core

out vec4 color;
void main()
{             
    float a = gl_FragCoord.z;
    float b = a * a;
    float c = a * b;
    float d = b * b;
    color = vec4(a, b, c, d);
    // gl_FragDepth = gl_FragCoord.z;
}