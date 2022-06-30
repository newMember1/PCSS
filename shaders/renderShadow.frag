#version 430 core
uniform sampler2D shadowMap;

in vec4 lightSpacePos;
out vec4 gColor;

void main()
{
    vec4 tLightSpacePos = lightSpacePos;
    tLightSpacePos.xyz = tLightSpacePos.xyz / tLightSpacePos.w;
    vec4 dep = texture(shadowMap, tLightSpacePos.xy / 2.0 + 0.5);

    vec3 baseColor = vec3(0.1, 0.3, 0.5);

    if(dep.x + 0.1< tLightSpacePos.z)
        gColor = vec4(baseColor * 0.1 , 1.0);
    else
        gColor = vec4(baseColor, 1.0);
}
