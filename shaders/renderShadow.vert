#version 430 core
layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 worldPos;

void main()
{
    gl_Position = projection * view * model * vec4(pos, 1.0f);
    worldPos = model * vec4(pos, 1.0);
}