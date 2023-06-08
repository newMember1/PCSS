#version 430 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 lightSpaceMatrix;
uniform vec3 viewPos;

out vec3 outNormal;
out vec4 lightSpacePos;
out vec3 viewDir;

void main()
{
    vec3 worldPos = (model * vec4(pos, 1.0f)).xyz;
    gl_Position = projection * view * vec4(worldPos, 1.0f);

    outNormal = vec3(model * vec4(normal, 1.0));
    lightSpacePos = lightSpaceMatrix * model * vec4(pos, 1.0);
    viewDir = viewPos - worldPos;
}