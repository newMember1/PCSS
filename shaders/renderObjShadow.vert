#version 430 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 lightSpaceMatrix;

out vec3 outNormal;
out vec4 lightSpacePos;

void main()
{
    gl_Position = projection * view * model * vec4(pos, 1.0f);

    outNormal = vec3(model * view * vec4(normal, 1.0));
    lightSpacePos = lightSpaceMatrix * model * vec4(pos, 1.0);
}