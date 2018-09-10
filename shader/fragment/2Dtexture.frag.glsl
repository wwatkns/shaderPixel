#version 400 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in float Near;
in float Far;

uniform float uTime;
uniform mat4 lightSpaceMat;
uniform vec3 cameraPos;
uniform sampler2D shadowMap;
uniform sampler2D noiseSampler;

float noise( vec3 x ) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
    vec2 uv = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
    vec2 rg = texture(noiseSampler, (uv + 0.5) / 256.0, 0.0).rg;
    return mix(rg.y, rg.x, f.z);
}

float fbm2d(in vec2 st, in float amplitude, in float frequency, in int octaves, in float lacunarity, in float gain) {
    float value = 0.0;
    st *= frequency;
    for (int i = 0; i < octaves; i++) {
        value += amplitude * noise(vec3(st, 1.0));
        st *= lacunarity;
        amplitude *= gain;
    }
    return value;
}


float   computeMeshShadows( vec3 hit, vec3 normal ) {
    vec4 posLightSpace = lightSpaceMat * vec4(hit, 1.0);
    vec3 projCoords = (posLightSpace.xyz / posLightSpace.w) * 0.5 + 0.5;
    float bias = 0.0025;
    /* Default */
    // float closestDepth = texture(shadowMap, projCoords.xy).r;
    // float shadow = (projCoords.z - bias > closestDepth ? 1.0 : 0.0);
    /* PCF */
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (projCoords.z - bias > pcfDepth ? 1.0 : 0.0);
        }
    }
    return (projCoords.z > 1.0 ? 0.0 : shadow / 9.0);
}

void    main() {
    vec2 uv = vec2(TexCoords.x, 1.0 - TexCoords.y);
    vec3 ndc = vec3(uv * 2.0 - 1.0, -1.0);

    float width = 0.5;
    float cells = 64.0;
    vec2 tex = floor(TexCoords.xy * cells) / cells;
    vec2 pos = fract(TexCoords.xy * cells);
    float xb = -1.0;
    float yb = fbm2d(vec2(TexCoords.x*2.0, tex.y-0.002+uTime*0.3) + fbm2d(vec2(TexCoords.x*2.0, tex.y-0.002-uTime*0.1), 1.5, 8.0, 10, 1.0, 0.53) * 0.33, 1.5, 4.0, 10, 1.0, 0.53) * 0.33;
    float r = float(abs(pos.y-width*0.5-(1.0-width)*yb) < width*0.5 || abs(pos.x-width*0.5-(1.0-width)*xb) < width*0.5);
    float b = (r >= 1.0 ? max(r-yb, 0.005) : 0.0);

    vec3 shadow = vec3(1.0 - computeMeshShadows(FragPos, vec3(-1.0, 0., 0.)))*0.5+0.5;

    FragColor = vec4(vec3(b) * shadow, 1.0);
}
