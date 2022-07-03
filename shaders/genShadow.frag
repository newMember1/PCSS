#version 330 core

in float dep;
out vec4 color;

void main()
{             
    float a = gl_FragCoord.z;
    float b = a * a;
    float c = a * b;
    float d = b * b;
    color = vec4(a, b, c, d);
}