#version 450 core
layout(location = 0) in vec3 pos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

out float dep;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(pos, 1.0);
    dep = gl_Position.z;
}