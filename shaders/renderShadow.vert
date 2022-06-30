#version 430 core
layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 lightSpaceMatrix;

out vec4 lightSpacePos;

void main()
{
    gl_Position = projection * view * model * vec4(pos, 1.0f);

    lightSpacePos = lightSpaceMatrix * model * vec4(pos, 1.0);
}