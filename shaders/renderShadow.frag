#version 430 core
uniform sampler2D shadowMap;
uniform vec3 baseColor;

in vec4 lightSpacePos;
out vec4 gColor;

//add PCF
float PCF(int filterSize, vec2 inShadowSamplePos, float depth)
{
    float shadow = 0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for(int i = -filterSize; i < filterSize; ++i)
    {
        for(int j = -filterSize; j < filterSize; ++j)
        {
            vec2 offset = vec2(i, j) * texelSize;
            vec2 shadowSamplePos = offset + inShadowSamplePos;

            float texelDep = texture(shadowMap, shadowSamplePos).r;

            if(texelDep + 0.02 < depth)
                shadow += 1.0;
        }
    }

    return shadow / (filterSize * filterSize);
}

void main()
{
    vec4 tLightSpacePos = lightSpacePos;
    float currentDep = tLightSpacePos.z / 2.0 + 0.5;
    
    // vec4 dep = texture(shadowMap, tLightSpacePos.xy / 2.0 + 0.5);
    float shadow = PCF(3, tLightSpacePos.xy / 2.0 + 0.5, currentDep);
    gColor = (1 - shadow) * vec4(baseColor, 1.0);
}
