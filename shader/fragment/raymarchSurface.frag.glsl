#version 400 core
out vec4 FragColor;

struct sDirectionalLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    bool use_shadows;
};

in vec3 FragPos;
in vec2 TexCoords;
in float Near;
in float Far;

uniform bool use_shadows;
uniform samplerCube skybox;
uniform sampler2D noiseSampler;

uniform sDirectionalLight directionalLight;
uniform mat4 invProjection;
uniform mat4 invView;
uniform mat4 lightSpaceMat;

uniform float uTime;
uniform vec3 cameraPos;

const int 	maxRaySteps = 196;      // the maximum number of steps the raymarching algorithm is allowed to perform
const float maxDist = 42.0;         // the maximum distance the ray can travel in world-space
const float minDist = 2./1080.;     // the distance from object threshold at which we consider a hit in raymarching

const int 	maxRayStepsShadow = 64; // the maximum number of steps the raymarching algorithm is allowed to perform for shadows
const float maxDistShadow = 3.0;    // the maximum distance the ray can travel in world-space
const float minDistShadow = 0.005;  // the distance from object threshold at which we consider a hit in raymarching for shadows

/* prototypes */
vec3    getNormal( in vec3 p );
vec3    computeDirectionalLight( in vec3 hit, in vec3 normal, in vec3 viewDir, in vec3 m_diffuse, bool use_shadows );
float   softShadow( in vec3 ro, in vec3 rd, float mint, float k );

// float   random(float x) {
//     return fract(sin(mod(x, 3.14))*43758.5453);
// }

float   random(vec2 p) {
    return fract(sin(mod(dot(p, vec2(12.9898,78.233)), 3.14))*43758.5453);
}

// float   random(vec3 p) {
// 	return fract(sin( mod(dot(p, vec3(113.5,271.9,124.6)), 3.14) )*43758.5453);
// }

vec2    random2( vec2 p ) {
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

// see https://shadertoyunofficial.wordpress.com/2016/07/20/special-shadertoy-features/ for fake interpolated 3D noise texture explanation.
float noise( vec3 x ) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
    vec2 uv = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
    vec2 rg = texture(noiseSampler, (uv + 0.5) / 256.0, 0.0).rg;
    return mix(rg.y, rg.x, f.z);
}

vec3    elevationMap(vec3 p, float size, float width) {
    vec3 f = abs(fract(p * size)-0.5);
    vec3 df = fwidth(p * size);
    float mi = max(0.0, width-1.0), ma = max(1.0, width);
    return clamp((f-df*mi)/(df*(ma-mi)), max(0.0, 1.0-width), 1.0);
}

float   fbm2d(in vec2 st, in float amplitude, in float frequency, in int octaves, in float lacunarity, in float gain) {
    float value = 0.0;
    st *= frequency;
    for (int i = 0; i < octaves; i++) {
        value += amplitude * noise(vec3(st, 1.0));
        st *= lacunarity;
        amplitude *= gain;
    }
    return value;
}

float   voronoi2d(vec2 uv, float scale) {
    uv *= scale;
    // Tile the space
    vec2 i_st = floor(uv);
    vec2 f_st = fract(uv);
    float m_dist = 1.; // minimun distance
    for (int y= -1; y <= 1; y++) {
        for (int x= -1; x <= 1; x++) {
            vec2 neighbor = vec2(float(x),float(y));     // Neighbor place in the grid
            vec2 point = random2(i_st + neighbor);       // Random position from current + neighbor place in the grid
            point = 0.5 + 0.5*sin(uTime + 6.2831*point); // Animate the point
            vec2 diff = neighbor + point - f_st;         // Vector between the pixel and the point
            float dist = length(diff);                   // Distance to the point
            m_dist = min(m_dist, dist);                  // Keep the closer distance
        }
    }
    return m_dist;
}

float   map2(vec2 p) {
    return fbm2d(p + (0.8-fbm2d(p + uTime * 0.025, 1.0, 1.0, 6, 2.2, 0.5)*1.3), 0.3, 1.0, 5, 1., 0.5);
}

float   map(vec3 p) {
    float h = fbm2d(p.xz + (0.8-fbm2d(p.xz + uTime * 0.025, 1.0, 1.0, 6, 2.2, 0.5)*1.3), 0.3, 1.0, 5, 1., 0.5);
    return p.y - h;
}

vec3    getNormal(vec3 p, float sphereR) {
	vec2 j = vec2(sphereR, 0.0);
	vec3 nor  	= vec3(0.0,		map2(p.xz), 0.0);
	vec3 v2		= nor-vec3(j.x,	map2(p.xz+j), 0.0);
	vec3 v3		= nor-vec3(0.0,	map2(p.xz-j.yx), -j.x);
	nor = cross(v2, v3);
	return normalize(nor);
}

vec3    getPixel(vec2 uv) {
    vec3 ndc = vec3(uv * 2.0 - 1.0, -1.0);
    vec3 dir = normalize(FragPos - cameraPos);
    /* relief */
    vec3 pos = (vec3(FragPos.x - uTime * 0.7, FragPos.yz) + 7.0)* .125;

	float t = minDist * random(gl_FragCoord.xy/256.);
	vec3 p = vec3(0.0);
    int i;
	for (i = 0; i < maxRaySteps; ++i) {
		if (t > maxDist) break;
		p = pos + dir * t;
		float h = map(p);
        h = h * 0.5 + t * .006;
        if (h < minDist)
            break;
		t += h;
	}
    vec3 sky = texture(skybox, dir).rgb + (i / float(maxRaySteps)) * vec3(1.0, 0.4815, 0.078) * 1.5;

    vec3 normal = getNormal(p, 0.025);
    vec3 light = computeDirectionalLight(p, normal, -dir, vec3(1.0), use_shadows);

    vec3 color = vec3(0.48, 0.27, 0.12) * clamp(p.y + 0.3, 0.0, 1.0) + // dirt
                 vec3( 1.0,  1.0,  1.0) * clamp(p.y - 0.3, 0.0, 1.0); // snow

    vec3 fog = (light*0.75+0.25) * (i / float(maxRaySteps)) * vec3(1.0, 0.2815, 0.078) * 0.95;
    return mix(light * color + fog, sky, clamp(t / maxDist, 0.0, 1.0) );
}

void    main() {
    vec2 uv = vec2(TexCoords.x, 1.0 - TexCoords.y);
    FragColor = vec4(getPixel(uv), 1.0);
}

vec3    computeDirectionalLight( in vec3 hit, in vec3 normal, in vec3 viewDir, in vec3 m_diffuse, bool use_shadows ) {
    vec3 lightDir = normalize(directionalLight.position);
    /* diffuse */
    float diff = max(dot(normal, lightDir), 0.0);
    /* specular */
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 12.0); // shininess
    /* shadow and ambient occlusion */
    float shadow = (use_shadows == true ? softShadow(hit, lightDir, 0.1, 16)*0.8+0.2 : 1.0);
    /* compute terms */
    vec3 ambient  = directionalLight.ambient  * m_diffuse;
    vec3 diffuse  = directionalLight.diffuse  * diff * m_diffuse;
    vec3 specular = directionalLight.specular * spec * vec3(0.2); // specular

    return ambient + (diffuse + specular) * shadow;
}

float   softShadow( in vec3 ro, in vec3 rd, float mint, float k ) {
    float t = mint;
    float res = 1.0;
	for (int i = 0; i < maxRayStepsShadow; ++i) {
        float h = map(ro + rd * t);
		if (h < minDistShadow)
            return 0.0;
        res = min(res, k * h / t);
		t += h;
        if (t > maxDistShadow) break;
	}
	return res;
}