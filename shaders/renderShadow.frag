#version 430 core
uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;

in vec4 worldPos;
out vec4 gColor;

void main()
{
    vec4 lightSpacePos = lightSpaceMatrix * worldPos;
    vec4 dep = texture(shadowMap, lightSpacePos.xy);

    gColor = dep;
}
