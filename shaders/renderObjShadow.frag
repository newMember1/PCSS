#version 430 core
uniform sampler2D shadowMap;
uniform vec3 baseColor;
uniform ivec2 LIGHTSIZE;   //pixel size or real radius? maybe radius is suitable
uniform vec3 lightDir;

in vec3 viewDir;
in vec3 outNormal;
in vec4 lightSpacePos;
out vec4 gColor;

const float ZNEAR = 0.1;

//quarter light size
// ivec2 LIGHTSIZE = ivec2(5, 5);

float BisaedZ(float dep)
{
    return dep + 0.02;
}

//PCF search neighbor pixel to estimate shadow ratio
float PCF(ivec2 filterSize, vec2 uv, float depth)
{
    float shadow = 0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for(int i = -filterSize.x; i < filterSize.x; ++i)
    {
        for(int j = -filterSize.y; j < filterSize.y; ++j)
        {
            vec2 offset = vec2(i, j) * texelSize;
            vec2 sampleUV = offset + uv;

            float texelDep = texture(shadowMap, sampleUV).r;

            if(BisaedZ(texelDep) < depth)
                shadow += 1.0;
        }
    }

    return 1.0 * shadow / (4 * filterSize.x * filterSize.y);
}

//use similar triangle to estimate search region
ivec2 SearchRegionRadius(float dReceiver)
{
    vec2 res = LIGHTSIZE * (dReceiver - ZNEAR) / dReceiver;
    return ivec2(res) + 1;
}

void FindBlocker(out float accuBlockDepth, out int numBlockers, float dReceiver, vec2 uv, ivec2 searchSize)
{
    accuBlockDepth = 0;
    numBlockers = 0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for(int i = -searchSize.x; i < searchSize.x; ++i)
    {
        for(int j = -searchSize.y; j < searchSize.y; ++j)
        {
            vec2 offset = vec2(i, j) * texelSize;
            vec2 sampleUV = offset + uv;
            float texelDep = texture(shadowMap, sampleUV).r;

            if(BisaedZ(texelDep) < dReceiver)
            {
                numBlockers += 1;
                accuBlockDepth += texelDep;
            }
        }
    }
}

ivec2 PenumbraRadius(float dBlocker, float dReceiver)
{
    vec2 radius = LIGHTSIZE * (dReceiver - dBlocker) / dBlocker;
    return ivec2(radius) + 1;
}

ivec2 ProjToLightUV(ivec2 sizeUV, float dReceiver)
{
    vec2 size = sizeUV * ZNEAR / dReceiver;
    return ivec2(size) + 1;
}

//here we use ortho projection for light thus one dReceiver should fit our solution
float PCSS(vec2 uv, float dReceiver)
{
    //1. do blocker search
    float accBlockDepth = 0.0f;
    int numBlockers = 0;
    //1.1 estimate search region, get quarter region size
    ivec2 searchRegionRadius = SearchRegionRadius(dReceiver);
    //1.2 search the region and find the average block depth
    FindBlocker(accBlockDepth, numBlockers, dReceiver, uv, searchRegionRadius);
    float aveBlockDepth = accBlockDepth / numBlockers;

    //2. calculate penumbra, use similiar triangle
    ivec2 penumbraRadius = PenumbraRadius(aveBlockDepth, dReceiver);
    ivec2 filterSize = ProjToLightUV(penumbraRadius, dReceiver);

    //3. PCF
    return PCF(penumbraRadius, uv, dReceiver);
}

void main()
{
    vec4 lsPos = lightSpacePos;
    float dReceiver = lsPos.z / 2.0 + 0.5; // rerange to [0, 1]
    
    // float shadow = PCF(LIGHTSIZE, lsPos.xy / 2.0 + 0.5, dReceiver);
    float shadow = PCSS(lsPos.xy / 2.0 + 0.5, dReceiver);

    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(outNormal, halfwayDir), 0.0), 2.0);

    gColor = (1 - shadow) * vec4(baseColor * spec + baseColor * vec3(0.2), 1.0);
}
