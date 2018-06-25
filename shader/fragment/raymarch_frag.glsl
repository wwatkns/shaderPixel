#version 400 core
out vec4 FragColor;

struct sDirectionalLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    bool use_shadows;
};

struct sMaterial {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    float opacity;
};

struct sObject {
    int         id;
    float       scale;
    mat4        invMat;
    sMaterial   material;
};

in vec3 FragPos;
in vec2 TexCoords;
in float Near;
in float Far;

#define MAX_OBJECTS 8

// uniform sampler2D noise;
uniform sampler2D depthBuffer;
uniform samplerCube skybox;
uniform sObject object[MAX_OBJECTS];
uniform int nObjects;

uniform sDirectionalLight directionalLight;
uniform mat4 invProjection;
uniform mat4 invView;
uniform vec2 uMouse;
uniform float uTime;
uniform vec3 cameraPos;

const int 	maxRaySteps = 300;      // the maximum number of steps the raymarching algorithm is allowed to perform
const float maxDist = 6.0;         // the maximum distance the ray can travel in world-space
const float minDist = 0.0001;        // the distance from object threshold at which we consider a hit in raymarching

const int 	maxRayStepsShadow = 64; // the maximum number of steps the raymarching algorithm is allowed to perform for shadows
const float maxDistShadow = 3.0;    // the maximum distance the ray can travel in world-space
const float minDistShadow = 0.005;  // the distance from object threshold at which we consider a hit in raymarching for shadows

#define OCCLUSION_ITERS 10
#define OCCLUSION_STRENGTH 32.0
#define OCCLUSION_GRANULARITY 0.05

/* prototypes */
vec4    raymarch( in vec3 ro, in vec3 rd, float s );
// float   raymarchVolume( in vec3 ro, in vec3 rd, float ct, float s );
vec3    getNormal( in vec3 p );
vec3    computeDirectionalLight( int objId, in vec3 hit, in vec3 normal, in vec3 viewDir, in vec3 m_diffuse, bool use_shadows, bool use_occlusion );
float   softShadow( in vec3 ro, in vec3 rd, float mint, float k );
float   ambientOcclusion( in vec3 hit, in vec3 normal );
vec3    map( in vec3 p );
float   fbm2d(in vec2 st, in float amplitude, in float frequency, in int octaves, in float lacunarity, in float gain);
float   fbm3d(in vec3 st, in float amplitude, in float frequency, in int octaves, in float lacunarity, in float gain);

vec2    raySphere( in vec3 ro, in vec3 rd, in vec4 sph, float dbuffer );
float   sphere( vec3 p, float s );
float   cube( vec3 p );
float   box( vec3 p, vec3 s );
float   smoothBox( vec3 p, vec3 s, float r );
float   torus( vec3 p );
vec2    mandelbulb( vec3 p );
vec2    mandelbox( vec3 p );
float   ifs( vec3 p );

float random (float x) {
    return fract(sin(mod(x, 3.14))*43758.5453);
}

float random(vec2 p) {
    return fract(sin(mod(dot(p, vec2(12.9898,78.233)), 3.14))*43758.5453);
}

float random(vec3 p) { // find better random ?
	return fract(sin( mod(dot(p, vec3(113.5,271.9,124.6)), 3.14) )*43758.5453);
}

float noise2d(in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

// http://www.iquilezles.org/www/articles/morenoise/morenoise.htm
vec4 noise3d(in vec3 x) {
    vec3 p = floor(x);
    vec3 w = fract(x);
    
    vec3 u = w*w*w*(w*(w*6.0-15.0)+10.0);
    vec3 du = 30.0*w*w*(w*(w-2.0)+1.0);

    float a = random(p + vec3(0.0,0.0,0.0));
    float b = random(p + vec3(1.0,0.0,0.0));
    float c = random(p + vec3(0.0,1.0,0.0));
    float d = random(p + vec3(1.0,1.0,0.0));
    float e = random(p + vec3(0.0,0.0,1.0));
    float f = random(p + vec3(1.0,0.0,1.0));
    float g = random(p + vec3(0.0,1.0,1.0));
    float h = random(p + vec3(1.0,1.0,1.0));

    float k0 = a;
    float k1 = b - a;
    float k2 = c - a;
    float k3 = e - a;
    float k4 = a - b - c + d;
    float k5 = a - c - e + g;
    float k6 = a - b - e + f;
    float k7 = -a + b + c - d + e - f - g + h;

    return vec4(-1.0+2.0*(k0 + k1*u.x + k2*u.y + k3*u.z + k4*u.x*u.y + k5*u.y*u.z + k6*u.z*u.x + k7*u.x*u.y*u.z), 
                2.0 * du * vec3( k1 + k4*u.y + k6*u.z + k7*u.y*u.z,
                                k2 + k5*u.z + k4*u.x + k7*u.z*u.x,
                                k3 + k6*u.x + k5*u.y + k7*u.x*u.y ) );
}

float pattern(vec2 st, vec2 v, float t) {
    vec2 p = floor(st+v);
    return step(t, random(100.+p*.000001)+random(p.x)*0.5 );
}

highp float hash( float n ) {
    return fract(sin(mod(n,3.14))*43758.5453);
}

float noise( in vec3 x ) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0 + 113.0*p.z;
    float res = mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                        mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
                    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                        mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
    return res;
}

// float noise( vec3 x ) { // iq's 3D noise from texture
//     vec3 f = fract(x);
//     vec3 p = x - f;
//     f = f*f*(3.0 - 2.0*f);
//     vec2 uv = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
//     vec2 rg = texture(noise, (uv + 0.5)/256.0, -100.0).rg;
//     return mix(rg.y, rg.x, f.z);
// }

mat3 m = mat3( 0.00,  0.80,  0.60,
              -0.80,  0.36, -0.48,
              -0.60, -0.48,  0.64 );

// float fbm( vec3 p ) {
//     float f;
//     f  = 0.5000*noise(p); p = m*p*2.02;
//     f += 0.2500*noise(p); p = m*p*2.03;
//     f += 0.1250*noise(p);
//     return f;
// }

// cloud
// vec4    raymarchVolume( in vec3 ro, in vec3 rd, in vec2 bounds, float radius, float s) { // same as raymarch, but we accumulate value when inside and sample the occlusion
//     bounds *= radius;
//     const int maxVolumeSamples = 50; // max steps in sphere
//     float stepSize = (2.0 * radius) / float(maxVolumeSamples); // granularity
// 	float t = bounds.x + stepSize * random(TexCoords.xy); // random dithering
//     vec4 sum = vec4(0.0);
// 	for (int i = 0; i < maxVolumeSamples; i++) {
//         if (sum.a > 0.99 || t > bounds.y || t > maxDist || t > s) break; // optimization and geometry occlusion
//         vec3 pos = ro + rd * t;
//         float se = fbm3d(pos, 0.65, 2.0, 5, 2.0, 0.35);
//         se = 1.0 / exp(se * 7.0);
//         se *= 1.0 - smoothstep(0.8 * radius, radius, length(pos)); // edge so that we have no interaction with sphere bounds
//         vec4 col = vec4(vec3(1.0-se), se);
// 		col.a *= 0.5;
// 		col.rgb *= col.a;
//         sum = sum + col * (1.0 - sum.a) * (stepSize * 100.0);
// 		t += stepSize;
// 	}
// 	return sum;
// }

// ADDING SHADING
// vec4    raymarchVolume( in vec3 ro, in vec3 rd, in vec2 bounds, float radius, float s) { // same as raymarch, but we accumulate value when inside and sample the occlusion
//     bounds *= radius;
//     // NEW
//     const int maxShadowSamples = 10;
//     float shadowStepSize = (2.0 * radius) / float(maxShadowSamples); 
//     vec3 lightVector = normalize(directionalLight.position) * shadowStepSize;
//     float absorption = 100.0;
//     const float r = 7.0;

//     const int maxVolumeSamples = 40; // max steps in sphere
//     float stepSize = (2.0 * radius) / float(maxVolumeSamples); // granularity
// 	float t = bounds.x + stepSize * random(TexCoords.xy); // random dithering
//     vec4 sum = vec4(0.0);

//     for (int i = 0; i < maxVolumeSamples; i++) {
//         if (sum.a > 0.999 || t > bounds.y || t > maxDist || t > s) break; // optimization and geometry occlusion
//         vec3 pos = ro + rd * t;
//         float se = fbm3d(pos, 0.43, 2.2, 4, 1.7, 0.55);
//         se = exp(-se * r);
//         se *= 1.0 - smoothstep(0.8 * radius, radius, length(pos)); // edge so that we have no interaction with sphere bounds
        
//         // SHADOW
//         float T1 = 1.0;
//         if (se > 0.001) {
//             for (int s = 0; s < maxShadowSamples; s++) {
//                 vec3 lpos = pos + lightVector * float(s);
//                 float ldensity = fbm3d(lpos, 0.43, 2.2, 4, 1.7, 0.55);
//                 ldensity = (1.0 - exp(-ldensity * 2.0 * r)) * exp(-ldensity * r);
//                 ldensity *= 1.0 - smoothstep(0.8 * radius, radius, length(lpos));

//                 if (ldensity > 0.0)
//                     T1 *= 1.0 - ldensity * absorption/float(maxVolumeSamples);
//                 if (T1 <= 0.01)
//                     break;
//             }
//         }

//         vec4 col = vec4(vec3(1.0-se), se);
// 		col.a *= 0.5;
//         col.rgb -= (1.0 - vec3(0.145, 0.431, 1.0)*0.5)*(1.0 - T1); // handle shadows
//         col.rgb *= min(T1*3.0, 1.0);
// 		col.rgb *= col.a;
//         sum = sum + col * (1.0 - sum.a) * (stepSize * 100.0);
// 		t += stepSize;
// 	}
// 	return sum;
// }

vec4    raymarchVolume( in vec3 ro, in vec3 rd, in vec2 bounds, float radius, float s) { // same as raymarch, but we accumulate value when inside and sample the occlusion
    bounds *= radius;
    // NEW
    const int maxShadowSamples = 10;
    float shadowStepSize = (2.0 * radius) / float(maxShadowSamples); 
    vec3 lightVector = normalize(directionalLight.position) * shadowStepSize;
    float absorption = 100.0;
    const float r = 7.0;
    
    const int maxVolumeSamples = 40; // max steps in sphere
    float stepSize = (2.0 * radius) / float(maxVolumeSamples); // granularity
	float t = bounds.x + stepSize * random(TexCoords.xy); // random dithering
    vec4 sum = vec4(0.0);

    for (int i = 0; i < maxVolumeSamples; i++) {
        if (sum.a > 0.999 || t > bounds.y || t > maxDist || t > s) break; // optimization and geometry occlusion
        vec3 pos = ro + rd * t;
        float se = fbm3d(pos + uTime*0.1, 0.5, 2.5, 6, 2.7, 0.38);
        se = exp(-se * r);
        se *= 1.0 - smoothstep(0.8 * radius, radius, length(pos)); // edge so that we have no interaction with sphere bounds
        
        // SHADOW
        float T1 = 1.0;
        for (int s = 0; s < maxShadowSamples; s++) {
            vec3 lpos = pos + lightVector * float(s);
            float ldensity = fbm3d(lpos + uTime*0.1, 0.5, 2.5, 6, 2.7, 0.38);
            // ldensity = exp(-ldensity * r);
            ldensity = (1.0 - exp(-ldensity * 2.0 * r)) * exp(-ldensity * r);
            ldensity *= 1.0 - smoothstep(0.8 * radius, radius, length(lpos));

            if (ldensity > 0.0)
                T1 *= 1.0 - ldensity*absorption/float(maxVolumeSamples);
            if (T1 <= 0.01 || T1 > 0.999)
                break;
        }

        // col.rgb -= (1.0 - vec3(0.145, 0.431, 1.0)*0.5)*(1.0 - T1); // handle shadows

        vec4 col = vec4(mix(vec3(1.0), vec3(directionalLight.ambient), se), se);
        col.rgb *= mix(vec3(directionalLight.ambient), vec3(1.0, 0.87, 0.64)*1.2, min(T1, 1.0));
		col.a *= 0.5;
		col.rgb *= col.a;
        sum = sum + col * (1.0 - sum.a) * (stepSize * 100.0);
		t += stepSize;
	}
	return sum;
}

// marble
// vec4    raymarchVolumeMarble( in vec3 ro, in vec3 rd, in vec2 bounds, float radius, float s ) {
//     bounds *= radius;
//     const int maxVolumeSamples = 500; // max steps in sphere
//     float stepSize = (2.0 * radius) / float(maxVolumeSamples); // granularity
// 	float t = bounds.x + stepSize * random(TexCoords.xy); // random dithering
//     vec4 sum = vec4(0.0);
// 	for (int i = 0; i < maxVolumeSamples; i++) {
//         if (sum.a > 0.99 || t > bounds.y || t > s) break; // optimization and geometry occlusion
//         vec3 pos = ro + rd * t;
//         float se = fbm3d(42.0 + pos * fbm3d(uTime*0.005 + pos, 0.5, 3.0, 4, 1.9, 0.5), 0.5, 3.0, 5, 3.0, 0.45);
//         se = 1.0 / exp(se * 5.0);
//         se *= 1.0 - smoothstep(0.9 * radius, radius, length(pos)); // prevent hard cut at sphere's bound
//         float v = exp(se*42.0)*0.0001;
//         vec4 col = vec4(vec3(se*se+v, se*se*v, se*se), se);
// 		col.a *= 0.75;
// 		col.rgb *= col.a;
//         col = clamp(col, 0.0, 1.0);
//         sum = sum + col * (2.0 - sum.a) * (stepSize * 100.0);
// 		t += stepSize;
// 	}
//     sum.rgb *= 3.5;
// 	return sum;
// }

void    main() {
    vec2 uv = vec2(TexCoords.x, 1.0 - TexCoords.y);
    vec3 ndc = vec3(uv * 2.0 - 1.0, -1.0);

    // direction is converted from ndc to world-space
    vec3 dir = (invProjection * vec4(ndc, 1.0)).xyz;
    dir = normalize((invView * vec4(dir, 0.0)).xyz);

    // convert depth buffer value to world depth
    float depth = texture(depthBuffer, uv).x * 2.0 - 1.0;
    depth = 2.0 * Near * Far / (Far + Near - depth * (Far - Near));

    // raymarch
    // vec4 res = raymarch(cameraPos, dir, depth);
    // if (res.x == 0.0) { FragColor = vec4(0.0); return ; }

    // int id = int(res.w);

    // compute useful variables for light
    // vec3 hit = cameraPos + dir * res.x;
    // vec3 normal = getNormal(hit);
    // vec3 viewDir = normalize(cameraPos - hit);

    // vec2 bounds = raySphere(cameraPos, dir, vec4(0.0, 0.0, 0.0, 1.0), depth);
    // vec3 hit = cameraPos + dir * bounds.x;
    // FragColor = vec4(vec3(1.0) * fbm3d(hit+uTime*0.015, 0.5, 10.0, 16, 4.0, 0.5), 1.0-fbm3d(hit+uTime*0.05, 0.5, 10.0, 16, 4.0, 0.5));
    // FragColor = vec4(vec3(1.0) * fbm3d(vec3(TexCoords.xy, uTime*0.05), 0.5, 10.0, 10, 2.5, 0.5), 1.0);
    // vec3 pos = vec3(FragPos.xy / vec2(1.0, 1.5), 0.0);
    // FragColor = vec4(vec3(1.0) * fbm3d(pos+uTime*0.1, 0.5, 2.0, 16, 2.5, 0.5), 1.0);
    // FragColor = vec4(vec3(1.0) * fbm(hit*2.0), 1.0);
    // return;
    
    // NON-VOLUMETRIC (MARBLE)
    // {
    //     vec4 sphere = vec4(0.0, 0.0, 0.0, 1.5);
    //     vec2 bounds = raySphere(cameraPos, dir, sphere, depth);
    //     if (bounds.x < 0.0) { FragColor = vec4(0.0); return ; }

    //     vec3 hit = cameraPos - sphere.xyz + dir * bounds.x * sphere.w;
    //     vec3 normal = normalize(hit);
    //     vec3 viewDir = -dir;

    //     vec4 color = raymarchVolumeMarble(cameraPos - sphere.xyz, dir, bounds, sphere.w, depth);
    //     color.xyz = clamp(vec3(0.0, 0.0, 0.03) + color.xyz, 0.0, 1.0);
    //     vec3 light = computeDirectionalLight(0, hit, normal, viewDir, color.xyz, false, false);
    //     // fresnel specular reflection
    //     if (bounds.x > 0.0) {
    //         vec3 spec = pow(texture(skybox, reflect(dir, hit)).rgb, vec3(2.2));
    //         float f = 1.0 - pow(1.0 - clamp(-dot(hit, dir), 0.0, 1.0), 4.0 / sphere.w);
    //         light = mix(spec, light, f);
    //     }
    //     FragColor = vec4(light, color.w + 0.8);
    //     return;
    // }

    /// VOLUMETRIC
    {
        vec4 sphere = vec4(0.0, 0.0, 0.0, 2.5);
        vec2 bounds = raySphere(cameraPos, dir, sphere, depth);
        if (bounds.x < 0.0) { FragColor = vec4(0.0); return ; }

        vec4 color = raymarchVolume(cameraPos - sphere.xyz, dir, bounds, sphere.w, depth);
        FragColor = color;
        return;
    }

    // compute colors
    // if (object[id].id == 0) { // mandelbox
    //     float it = res.z / float(maxRaySteps);
    //     vec3 color = (vec3(0.231, 0.592, 0.776) + res.y * res.y * vec3(0.486, 0.125, 0.125)) * 0.3;
    //     vec3 light = computeDirectionalLight(id, hit, normal, viewDir, vec3(0.898, 0.325, 0.7231), false, false);
    //     // NEW
    //     vec2 st = hit.xy * 10;
    //     vec2 grid = vec2(100.0, 50.0);
    //     st *= grid;
    //     vec2 ipos = floor(st);
    //     vec2 fpos = fract(st);
    //     vec2 vel = vec2(uTime * 0.3 * max(grid.x, grid.y));
    //     vel *= vec2(-1.0, 0.0) * random(1.0 + ipos.y);
    //     vec2 offset = vec2(0.1, 0.);
    //     vec3 patt = vec3(0.);
    //     patt.r = pattern(st + offset, vel, 0.5 + 0.1);
    //     patt.g = pattern(st, vel, 0.5 + 0.1);
    //     patt.b = pattern(st - offset, vel, 0.5 + 0.1);
    //     patt *= step(0.2, fpos.y);
    //     patt = (1.0-patt * 0.7) * vec3(1.0, 0.73, 0.3) * 1.5;

    //     // float r = 2.0*fbm2d(hit.xy + fbm2d(hit.xy + uTime*0.01, 0.85, 3.0, 16, 3.0, 0.5), 0.85, 3.0, 16, 6.0, 0.5);

    //     FragColor = vec4(patt * light * color * vec3(0.9, 0.517, 0.345) * 1.2, -log(it)*2.0);
    //     /* add fog if we're in cube */
    //     float fog = res.x * 0.5 / object[id].scale * 12.0;
    //     vec3 colorFog = fog * fog * vec3(0.9, 0.517, 0.345) * 0.5;
    //     float t = 0.0;
    //     for (int i = 0; i < 10; i++) {
    //         float scale = object[id].scale;
    //         vec3 p = (object[id].invMat * vec4(cameraPos + dir * t, 1.0)).xyz / scale;
    //         float res = cube(p) * scale;
    //         t += res;
    //         if (t > maxDist || t > depth) break;
    //         if (res < 0.1*object[id].scale) { FragColor.xyz += clamp(colorFog * (0.1-t), 0.0, 10.0); break; }
    //     }
    // }
    // else if (object[id].id == 1) { // mandelbulb
    //     res.y = pow(clamp(res.y, 0.0, 1.0), 0.55);
    //     vec3 tc0 = 0.5 + 0.5 * sin(3.0 + 4.2 * res.y + vec3(0.0, 0.5, 1.0));
    //     vec3 diffuse = vec3(0.9, 0.8, 0.6) * 0.2 * tc0 * 8.0;
    //     vec3 color = computeDirectionalLight(id, hit, normal, viewDir, diffuse, true, true);
    //     FragColor = vec4(color, object[id].material.opacity);
    // }
    // else if (object[id].id == 2) { // ifs
    //     float g = pow(2.0 + res.z / float(maxRaySteps), 4.0) * 0.05;
    //     vec3 glow = vec3(1.0 * g, 0.819 * g * 0.9, 0.486) * g * 2.0;
    //     vec3 diffuse = vec3(1.0, 0.694, 0.251);
    //     vec3 light = computeDirectionalLight(id, hit, normal, viewDir, diffuse, true, false);
    //     FragColor = vec4(light + log(glow * 0.95) * 0.75, 1.0);
    // }
    // else {
    //     vec3 color = computeDirectionalLight(id, hit, normal, viewDir, object[id].material.diffuse, true, true);
    //     FragColor = vec4(color, object[id].material.opacity);
    // }

    // iterations color
    // {
        // FragColor = vec4(res.z / float(maxRaySteps), 0, 0.03, 1.0);
    // }

    // depth-buffer debug
    // {
    //     float near = 0.1;
    //     float far = 100.0;
    //     float depth = texture(depthBuffer, uv).x;
    //     float c = (2.0 * near) / (far + near - depth * (far - near));
    //     FragColor = vec4(c, c, c, 1.0);
    // }
}

vec3    opU( vec3 d1, vec3 d2 ) {
	return (d1.x < d2.x) ? d1 : d2;
}

vec3    map( in vec3 p ) {
    vec3 pos = p;
    vec3 new;
    vec3 res = vec3(Far);
    for (int i = 0; i < MAX_OBJECTS && i < nObjects; i++) {
        pos = (object[i].invMat * vec4(p, 1.0)).xyz / object[i].scale;

        if (object[i].id == 0)
            new = vec3(mandelbox(pos), 0);
        else if (object[i].id == 1)
            new = vec3(mandelbulb(pos), 1);
        else if (object[i].id == 2)
            new = vec3(ifs(pos), 0, 2);
        else if (object[i].id == 3)
            new = vec3(sphere(pos, 1.0), 0, 3); // WIP
        else if (object[i].id == 4)
            new = vec3(torus(pos), 0, 4);

        new.x *= object[i].scale;
        new.z = i;
        res = opU(res, new);
    }
    return res;
}

/* return: distance, trap, ray_iterations, object_id */
vec4    raymarch( in vec3 ro, in vec3 rd, float s ) {
	float t = 0.0;
    ro += rd * minDist * random(gl_FragCoord.xy/256);
	for (int i = 0; i < maxRaySteps; i++) {
        vec3 res = map(ro + rd * t);
		t += res.x;
        if (t > maxDist || t > s) return vec4(0.0, 0.0, i, 0.0); // optimization and geometry occlusion
		if (res.x < minDist) return vec4(t, res.y, i, res.z);
	}
	return vec4(0.0, 0.0, maxRaySteps, 0.0);
}

vec3    getNormal( in vec3 p ) {
    const vec2 eps = vec2(minDist, 0.0);
    return normalize(vec3(map(p + eps.xyy).x - map(p - eps.xyy).x,
                          map(p + eps.yxy).x - map(p - eps.yxy).x,
                          map(p + eps.yyx).x - map(p - eps.yyx).x));
}

vec3    computeDirectionalLight( int objId, in vec3 hit, in vec3 normal, in vec3 viewDir, in vec3 m_diffuse, bool use_shadows, bool use_occlusion ) {
    vec3 lightDir = normalize(directionalLight.position);
    /* diffuse */
    float diff = max(dot(normal, lightDir), 0.0);
    /* specular */
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), object[objId].material.shininess);
    /* shadow and ambient occlusion */
    float shadow = (use_shadows == true ? softShadow(hit, lightDir, 0.1, 32) : 1.0);
    float ao = (use_occlusion == true ? ambientOcclusion(hit, normal) : 1.0);
    /* compute terms */
    vec3 ambient  = directionalLight.ambient  * m_diffuse;
    vec3 diffuse  = directionalLight.diffuse  * diff * m_diffuse;
    vec3 specular = directionalLight.specular * spec * object[objId].material.specular;
    return ambient + (diffuse + specular) * shadow * ao;
}

float   softShadow( in vec3 ro, in vec3 rd, float mint, float k ) {
    float t = mint;
    float res = 1.0;
	for (int i = 0; i < maxRayStepsShadow; ++i) {
        float h = map(ro + rd * t).x;
		if (h < minDistShadow)
            return 0.0;
        res = min(res, k * h / t);
		t += h;
        if (t > maxDistShadow) break;
	}
	return res;
}

float   ambientOcclusion( in vec3 hit, in vec3 normal ) {
    float k = 1.0;
    float d = 0.0;
    float occ = 0.0;
    for(int i = 0; i < OCCLUSION_ITERS; i++){
        d = map(hit + normal * k * OCCLUSION_GRANULARITY).x;
        occ += 1.0 / pow(3.0, k) * ((k - 1.0) * OCCLUSION_GRANULARITY - d);
        k += 1.0;
    }
    return 1.0 - clamp(occ * OCCLUSION_STRENGTH, 0.0, 1.0);
}

float fbm2d(in vec2 st, in float amplitude, in float frequency, in int octaves, in float lacunarity, in float gain) {
    float value = 0.0;
    st *= frequency;
    for (int i = 0; i < octaves; i++) {
        value += amplitude * noise2d(st);
        st *= lacunarity;
        amplitude *= gain;
    }
    return value;
}

float fbm3d(in vec3 st, in float amplitude, in float frequency, in int octaves, in float lacunarity, in float gain) {
    float value = 0.0;
    st *= frequency;
    for (int i = 0; i < octaves; i++) {
        value += amplitude * noise(st);
        st *= lacunarity;
        amplitude *= gain;
    }
    return value;
}

// returns the min/max dist if intersecting with the sphere (sph is a vec4 with xyz being pos and w the size)
vec2 raySphere( in vec3 ro, in vec3 rd, in vec4 sph, float dbuffer ) {
    float ndbuffer = dbuffer / sph.w;
    vec3  rc = (ro - sph.xyz) / sph.w;
    float b = dot(rd, rc);
    float c = dot(rc, rc) - 1.0;
    float h = b*b - c;
    if (h < 0.0) return vec2(-1.0);
    h = sqrt(h);
    float t1 = -b - h;
    float t2 = -b + h;
    if (t2 < 0.0 || t1 > ndbuffer) return vec2(-1.0);
    return vec2(max(t1, 0.0), min(t2, ndbuffer));
}

/*  Distance Estimators
*/

float   sphere( vec3 p, float s ) {
    return length(p) - s;
}

float   torus( vec3 p ) {
    vec2 t = vec2(2, 0.75);
    vec2 q = vec2(length(p.xz) - t.x, p.y);
    return length(q) - t.y;
}

float   cube( vec3 p ) {
    vec3 d = abs(p) - vec3(1, 1, 1);
    return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

float   box( vec3 p, vec3 s ) {
    vec3 d = abs(p) - s;
    return min(max(d.x, max(d.y, d.z)), 0.0) + length(max(d, 0.0));
}

float   smoothBox( vec3 p, vec3 s, float r ) {
    return length(max(abs(p) - s, 0.0)) - r;
}


vec2   mandelbulb( vec3 p ) {
    const float power = 8.0;
    vec3 z = p;
    float dr = 1.0;
    float r, theta, phi;
    float t0 = 1.0;
    for (int i = 0; i < 6; i++) {
        r = length(z);
        if (r > 2.0) break;
        // convert to polar coordinates
        theta = asin(z.z / r) + uTime * 0.1;
        phi = atan(z.y, z.x);// + uTime * 0.05;
        float rpow = pow(r, power - 1.0);
        dr = rpow * power * dr + 1.0;
        // scale and rotate the point
        r = rpow * r;
        theta = theta * power;
        phi = phi * power;
        // convert back to cartesian coordinates
        float cosTheta = cos(theta);
        z = r * vec3(cosTheta * cos(phi), sin(phi) * cosTheta, sin(theta)) + p;
        t0 = min(t0, r);
	}
	return vec2((0.5 * log(r) * r / dr), t0);
}

vec2   mandelbox( vec3 p ) {
    const float fold_limit = 1.0;
    const float fold_value = 2.0;
    const float min_radius = 0.5;
    const float scale = 2.0;
    const float minRadius2 = min_radius * min_radius;

    const vec4 scalevec = vec4(scale, scale, scale, abs(scale)) / minRadius2;
    const float C1 = abs(scale - 1.0), C2 = pow(abs(scale), float(1 - 12));

    vec4 z = vec4(p.xyz * 6.0, 1.0), p0 = vec4(p.xyz * 6.0, 1.0);
    float t0 = 1.0;
    for (int i = 0; i < 13; i++) {
        z.xyz = clamp(z.xyz, -1.0, 1.0) * 2.0 - z.xyz;  // box fold
        float r2 = dot(z.xyz, z.xyz);
        z.xyzw *= clamp(max(minRadius2 / r2, minRadius2), 0.0, 1.0);  // sphere fold
        z.xyzw = z * scalevec + p0;
        t0 = min(t0, r2);
    }
	return vec2(((length(z.xyz) - C1) / z.w - C2) / 6.0, t0);
}

float   ifs(vec3 p) {
    float ui = 100.0 * uTime * 0.1;
    float y = -0.001 * ui;
    mat2  m = mat2(sin(y), cos(y), -cos(y), sin(y));
    y = 0.0035 * ui;
    mat2  n = mat2(sin(y), cos(y), -cos(y), sin(y));
    y = 0.0023 * ui;
    mat2 nn = mat2(sin(y), cos(y), -cos(y), sin(y));

    float t = 1.0;
    for (int i = 0; i < 12; i++) {
        t = t * 0.66;
        p.xy =  m * p.xy;
        p.yz =  n * p.yz;
        p.zx = nn * p.zx;
        p.xz = abs(p.xz) - t;
    }
    // return sphere(p, 2.0 * t);
    return smoothBox(p, vec3(0.975 * t), 0.1 * t);
}
