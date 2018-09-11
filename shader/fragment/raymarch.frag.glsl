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
    float       boundingSphereScale;
    mat4        invMat;
    sMaterial   material;
};

in vec3 FragPos;
in vec2 TexCoords;
in float Near;
in float Far;

#define MAX_OBJECTS 8
#define OCCLUSION_ITERS 10
#define OCCLUSION_STRENGTH 32.0
#define OCCLUSION_GRANULARITY 0.05

uniform sampler2D depthBuffer;
uniform sampler2D shadowMap;
uniform bool use_shadows;
uniform samplerCube skybox;
uniform sampler2D noiseSampler;
uniform sObject object[MAX_OBJECTS];
uniform int nObjects;

uniform sDirectionalLight directionalLight;
uniform mat4 invProjection;
uniform mat4 invView;
uniform mat4 lightSpaceMat;

uniform vec2 uMouse;
uniform float uTime;
uniform vec3 cameraPos;

/* globals */
const int 	maxRaySteps = 128;      // the maximum number of steps the raymarching algorithm is allowed to perform
const float maxDist = 50.0;         // the maximum distance the ray can travel in world-space
const float minDist = 2./1080.;//0.001; // the distance from object threshold at which we consider a hit in raymarching

const int 	maxRayStepsShadow = 64; // the maximum number of steps the raymarching algorithm is allowed to perform for shadows
const float maxDistShadow = 3.0;    // the maximum distance the ray can travel in world-space
const float minDistShadow = 0.005;  // the distance from object threshold at which we consider a hit in raymarching for shadows

/* prototypes */
vec4    raymarch( in vec3 ro, in vec3 rd, float s );
vec4    raymarchObj( in vec3 ro, in vec3 rd, float s, int i );
vec3    getNormal( in vec3 p );
vec3    getNormalObj( in vec3 p, int i );
vec3    map( in vec3 p );
vec3    mapObj( in vec3 p, int i );
vec3    computeDirectionalLight( int objId, in vec3 hit, in vec3 normal, in vec3 viewDir, in vec3 m_diffuse, bool use_shadows, bool use_occlusion );
float   softShadow( in vec3 ro, in vec3 rd, float mint, float k );
float   ambientOcclusion( in vec3 hit, in vec3 normal );
float   fbm3d(in vec3 st, in float amplitude, in float frequency, in int octaves, in float lacunarity, in float gain);
vec2    raySphere( in vec3 ro, in vec3 rd, in vec4 sph, float dbuffer );
float   blob( vec3 p );
float   sphere( vec3 p, float s );
float   smoothBox( vec3 p, vec3 s, float r );
vec2    mandelbulb( vec3 p );
vec2    mandelbox( vec3 p );
float   ifs( vec3 p );

float random(vec2 p) {
    return fract(sin(mod(dot(p, vec2(12.9898,78.233)), 3.14))*43758.5453);
}

float noise( vec3 x ) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
    vec2 uv = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
    vec2 rg = texture(noiseSampler, (uv + 0.5) / 256.0, 0.0).rg;
    return mix(rg.y, rg.x, f.z);
}

vec4    raymarchVolume( in vec3 ro, in vec3 rd, in vec2 bounds, float radius, float s) { // same as raymarch, but we accumulate value when inside and sample the occlusion
    bounds *= radius;
    const int maxVolumeSamples = 35; // max steps for density sampling
    const int maxShadowSamples = 20; // max steps for shadow sampling
    const float r = 6.0;
    float shadowStepSize = (2.0 * radius) / float(maxShadowSamples);
    vec3 lightVector = normalize(directionalLight.position) * shadowStepSize;
    float absorption = 50.0 / float(maxShadowSamples);
    float stepSize = (2.0 * radius) / float(maxVolumeSamples); // granularity
    float t = bounds.x + stepSize * random(gl_FragCoord.xy); // random dithering
    vec4 sum = vec4(0.0);

    for (int i = 0; i < maxVolumeSamples; i++) {
        if (sum.a > 0.99 || t > bounds.y || t > maxDist || t > s) break; // optimization and geometry occlusion
        vec3 pos = ro + rd * t;
        float se = fbm3d(pos + uTime*0.1, 0.8, 2.0, 4, 2.7, 0.248);
        se = 1.0/exp(se * r);
        se *= 1.0 - smoothstep(0.75 * radius, radius, length(pos)); // edge so that we have no interaction with sphere bounds
        // Compute shadow from directional light source
        float T1 = 1.0;
        for (int s = 0; s < maxShadowSamples; s++) {
            vec3 lpos = pos + lightVector * float(s);
            if (lpos.x < -radius || lpos.x > radius || lpos.y < -radius || lpos.y > radius || lpos.z < -radius || lpos.z > radius) // optimization
                break;
            float ldensity = fbm3d(lpos + uTime*0.1, 0.8, 2.0, 4, 2.7, 0.248);
            ldensity = 1.0/exp(ldensity * r);
            ldensity *= 1.0 - smoothstep(0.75 * radius, radius, length(lpos));
            if (ldensity > 0.0)
                T1 *= clamp(1.0 - ldensity * absorption, 0.0, 1.0);
            if (T1 <= 0.01) break;
        }
        vec4 col = vec4(vec3(1.0) * max(T1, 0.15), se);
		col.a *= 0.4;
		col.rgb *= col.a;
        sum += col * (1.0 - sum.a) * (stepSize * 100.0);
		t += stepSize;
	}
	return sum;
}

vec4    raymarchVolumeMarble( in vec3 ro, in vec3 rd, in vec2 bounds, float radius, float s ) {
    bounds *= radius;
    const int maxVolumeSamples = 500; // max steps in sphere
    float stepSize = (2.0 * radius) / float(maxVolumeSamples); // granularity
	float t = bounds.x + stepSize * random(TexCoords.xy); // random dithering
    vec4 sum = vec4(0.0);
	for (int i = 0; i < maxVolumeSamples; i++) {
        if (sum.a > 0.99 || t > bounds.y || t > s) break; // optimization and geometry occlusion
        vec3 pos = ro + rd * t;
        float se = fbm3d(42.0 + pos * fbm3d(uTime*0.005 + pos, 0.5, 3.0, 4, 1.9, 0.5), 0.5, 3.0, 5, 3.0, 0.45);
        se = 1.0 / exp(se * 5.0);
        se *= 1.0 - smoothstep(0.9 * radius, radius, length(pos)); // prevent hard cut at sphere's bound
        float v = exp(se*42.0)*0.0001;
        vec4 col = vec4(vec3(se*se+v, se*se*v, se*se), se);
		col.a *= 0.75;
		col.rgb *= col.a;
        col = clamp(col, 0.0, 1.0);
        sum = sum + col * (2.0 - sum.a) * (stepSize * 100.0);
		t += stepSize;
	}
    sum.rgb *= 3.5;
	return sum;
}

// https://stackoverflow.com/questions/32227283/getting-world-position-from-depth-buffer-value
vec3    worldPosFromDepth( float depth, vec3 ndc ) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(ndc.xy, z, 1.0);
    vec4 viewSpacePosition = invProjection * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;
    vec4 worldSpacePosition = invView * viewSpacePosition;
    return worldSpacePosition.xyz;
}

vec3    sampleHemisphere(float u1, float u2, vec3 normal) {
	vec3 u = normal;
	vec3 v = abs(u.y) < abs(u.z) ? vec3(0.0, 1.0, 0.0) : vec3(0.0, 0.0, 1.0);
	vec3 w = normalize(cross(u, v));
	v = cross(w, u);

	float r = sqrt(u1);
	float theta = 2.0 * 3.1415926535 * u2;
	float x = r * cos(theta);
	float y = r * sin(theta);
	return normalize(u * sqrt(1.0 - u1) + v * x + w * y);
}

vec3    diffuseFromSkybox(vec3 p, vec2 n) {	
	vec3 col = vec3(0.0);
	for (float i = 0.0; i < 4.0; i++)
		for (float j = 0.0; j < 8.0; j++) {
			vec2 s = vec2(i, j)+n;
			float u = (random(p.xy+s)+i)*0.25;
			float v = (random(p.yz+s)+j)*0.125;
			vec3 ns = sampleHemisphere(u*0.5, v, p);
			col += pow(texture(skybox, ns).rgb, vec3(2.2));
		}
	return col * 0.25 * 0.125;
}

void    main() {
    vec2 uv = vec2(TexCoords.x, 1.0 - TexCoords.y);
    vec3 ndc = vec3(uv * 2.0 - 1.0, -1.0);

    /* direction is converted from ndc to world-space */
    vec3 dir = (invProjection * vec4(ndc, 1.0)).xyz;
    dir = normalize((invView * vec4(dir, 0.0)).xyz);

    /* depth-buffer value conversion to world space (for correct distance for) */
    float depth = texture(depthBuffer, uv).x;
    depth = distance(cameraPos, worldPosFromDepth(depth, ndc));

    /* Bounding sphere optimisation */
    vec4 res = vec4(0.0);
    for (int i = 0; i < MAX_OBJECTS && i < nObjects; i++) {
        if (object[i].id != 3 && object[i].id != 4) {
            vec3 pos = (object[i].invMat * vec4(vec3(0.0), -1.0)).xyz;
            vec4 sphere = vec4(pos, object[i].scale * object[i].boundingSphereScale);
            vec2 bounds = raySphere(cameraPos, dir, sphere, depth);
            if (bounds.x < 0.0) { continue ; }

            vec3 ro = cameraPos + dir * bounds.x;
            vec4 tmp = raymarchObj(ro, dir, depth, i);

            // res.z = tmp.z; // keep correct number of iterations for debug
            if (tmp.x > 0.0) {
                tmp.x += bounds.x;
                if (tmp.x < res.x || res.x == 0.0)
                    res = tmp;
            }
        }
    }
    // vec4 res = raymarch(cameraPos, dir, depth);

    /* Colorize the object */
    FragColor = vec4(0.0);
    if (res.x > 0.0) {
        depth = res.x; // used for volumetric raymarching after for collision
        int id = int(res.w);
        /* compute useful variables for light */
        vec3 hit = cameraPos + dir * res.x;
        vec3 normal = getNormalObj(hit, int(res.w));
        vec3 viewDir = -dir;

        /* compute colors */
        if (object[id].id == 0) { /* mandelbox */
            float it = res.z / float(maxRaySteps);
            vec3 color = (vec3(0.231, 0.592, 0.776) + res.y * res.y * vec3(0.486, 0.125, 0.125)) * 0.3;
            vec3 diffuse = vec3(0.898, 0.325, 0.7231) * 0.5;
            vec3 light = computeDirectionalLight(id, hit, normal, viewDir, diffuse, use_shadows, false);
            FragColor = vec4(light * color, object[id].material.opacity);
        }
        else if (object[id].id == 1) { /* mandelbulb */
            res.y = pow(clamp(res.y, 0.0, 1.0), 0.55);
            vec3 tc0 = 0.5 + 0.5 * sin(2.65 + 4. * res.y + vec3(.7, 1.0, 0.1));
            vec3 diffuse = vec3(0.9, 0.8, 0.6) * 0.5 * tc0 * 4.0;
            vec3 color = computeDirectionalLight(id, hit, normal, viewDir, diffuse, use_shadows, true);
            FragColor = vec4(color, object[id].material.opacity);
        }
        else if (object[id].id == 2) { /* IFS */
            float g = pow(2.0 + res.z / float(maxRaySteps), 4.0) * 0.05;
            vec3 glow = vec3(1.0 * g, 0.819 * g * 0.9, 0.486) * g * 2.0;
            vec3 diffuse = vec3(1.0, 0.694, 0.251);
            vec3 light = computeDirectionalLight(id, hit, normal, viewDir, diffuse, use_shadows, false);
            FragColor = vec4(light + log(glow * 0.95) * 0.75, object[id].material.opacity);
        }
        else { /* default */
            vec3 light = computeDirectionalLight(id, hit, normal, viewDir, object[id].material.diffuse, use_shadows, false);
            /* fresnel reflection */
            vec3 diff = diffuseFromSkybox(normal, vec2(sin(uTime))*FragPos.xy) + 0.15;
            vec3 spec = pow(texture(skybox, reflect(dir, normal)).rgb, vec3(2.2));
            float f = 1.0 - pow(1.0 - clamp(-dot(normal, dir), 0.0, 1.0), 5.0);
            light = mix(spec, diff * light, f);
            FragColor = vec4(light, object[id].material.opacity);
        }
    }

    /* Raymarching for volumetric objects */
    vec4 color = vec4(0.0);
    for (int i = 0; i < MAX_OBJECTS && i < nObjects; i++) {
        if (object[i].id == 3 || object[i].id == 4) { // 3 and 4 are marble and cloud
            vec3 pos = (object[i].invMat * vec4(vec3(0.0), -1.0)).xyz;
            vec4 sphere = vec4(pos, object[i].scale);
            vec2 bounds = raySphere(cameraPos, dir, sphere, depth);
            if (bounds.x < 0.0) { continue ; }

            if (object[i].id == 3) { /* Marble */
                vec3 hit = cameraPos - sphere.xyz + dir * bounds.x * sphere.w;
                vec3 normal = normalize(hit);
                vec3 viewDir = -dir;

                vec4 col = raymarchVolumeMarble(cameraPos - sphere.xyz, dir, bounds, sphere.w, depth);
                col.xyz = clamp(vec3(0.0, 0.0, 0.03) + col.xyz, 0.0, 1.0);
                vec3 light = computeDirectionalLight(i, hit + sphere.xyz, normal, viewDir, col.xyz, use_shadows, false);
                /* fresnel specular reflection */
                if (bounds.x > 0.0) {
                    vec3 spec = pow(texture(skybox, reflect(dir, normal)).rgb, vec3(2.2));
                    float f = 1.0 - pow(1.0 - clamp(-dot(normal, dir), 0.0, 1.0), 5.0 / sphere.w);
                    light = mix(spec, light, f);
                }
                if (col.w + 0.8 > 0.0)
                    color.rgb *= 1.0 - min(col.w + 0.8, 1.0);
                color += vec4(light, col.w + 0.8);
                depth = distance(cameraPos, sphere.xyz);
            }
            else if (object[i].id == 4) { /* Cloud */
                vec4 col = raymarchVolume(cameraPos - sphere.xyz, dir, bounds, sphere.w, depth);
                if (col.w > 0.0)
                    color.rgb *= 1.0 - min(col.w, 1.0);
                color += col;
            }
        }
    }
    if (color.w > 0.0)
        FragColor.xyz *= 1.0 - min(color.w, 1.0);
    FragColor += color;

    
    /* iterations color debug */
    // FragColor = vec4(res.z / float(maxRaySteps), 0, 0.03, 1.0);
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
            new = vec3(mandelbox(pos), 0.);
        else if (object[i].id == 1)
            new = vec3(mandelbulb(pos), 1.);
        else if (object[i].id == 2)
            new = vec3(ifs(pos), 0., 2.);
        else if (object[i].id == 5)
            new = vec3(blob(pos), 0., 5.);
        else
            new = vec3(100000., 0., 3.);

        new.x *= object[i].scale;
        new.z = i;
        res = opU(res, new);
    }
    return res;
}

/* return: distance, trap, ray_iterations, object_id */
vec4    raymarch( in vec3 ro, in vec3 rd, float s ) {
	float t = 0.0;
    ro += rd * minDist * random(gl_FragCoord.xy/256.);
	for (int i = 0; i < maxRaySteps; i++) {
        vec3 res = map(ro + rd * t);
		t += res.x;
        if (t > maxDist || t > s) return vec4(0.0, 0.0, i, 0.0); // optimization and geometry occlusion
		if (res.x < minDist) return vec4(t, res.y, i, res.z);
	}
	return vec4(0.0, 0.0, maxRaySteps, 0.0);
}

vec3    getNormal( in vec3 p ) {
    const vec2 eps = vec2(minDist * 0.5, 0.0);
    return normalize(vec3(map(p + eps.xyy).x - map(p - eps.xyy).x,
                          map(p + eps.yxy).x - map(p - eps.yxy).x,
                          map(p + eps.yyx).x - map(p - eps.yyx).x));
}

vec3    mapObj( in vec3 p, int i ) {
    vec3 pos = p;
    vec3 res = vec3(Far, 0., i);

    pos = (object[i].invMat * vec4(p, 1.0)).xyz / object[i].scale;
    if (object[i].id == 0)
        res.xy = mandelbox(pos);
    else if (object[i].id == 1)
        res.xy = mandelbulb(pos);
    else if (object[i].id == 2)
        res.x = ifs(pos);
    else if (object[i].id == 5)
        res.x = blob(pos);
    res.x *= object[i].scale;
    return res;
}

vec4    raymarchObj( in vec3 ro, in vec3 rd, float s, int obj ) {
	float t = 0.0;
    ro += rd * minDist * random(gl_FragCoord.xy/256.);
	for (int i = 0; i < maxRaySteps; i++) {
        vec3 res = mapObj(ro + rd * t, obj);
		t += res.x;
        if (t > maxDist || t > s) return vec4(0.0, 0.0, i, 0.0);
		if (res.x < minDist) return vec4(t, res.y, i, res.z);
	}
	return vec4(0.0, 0.0, maxRaySteps, 0.0);
}

vec3    getNormalObj( in vec3 p, int i ) {
    const vec2 eps = vec2(minDist * 0.5, 0.0);
    return normalize(vec3(mapObj(p + eps.xyy, i).x - mapObj(p - eps.xyy, i).x,
                          mapObj(p + eps.yxy, i).x - mapObj(p - eps.yxy, i).x,
                          mapObj(p + eps.yyx, i).x - mapObj(p - eps.yyx, i).x));
}

float   computeMeshShadows( vec3 hit, vec3 normal ) {
    vec4 posLightSpace = lightSpaceMat * vec4(hit, 1.0);
    vec3 projCoords = (posLightSpace.xyz / posLightSpace.w) * 0.5 + 0.5;
    float bias = 0.0025;
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

vec3    computeDirectionalLight( int objId, in vec3 hit, in vec3 normal, in vec3 viewDir, in vec3 m_diffuse, bool use_shadows, bool use_occlusion ) {
    vec3 lightDir = normalize(directionalLight.position);
    /* diffuse */
    float diff = max(dot(normal, lightDir), 0.0);
    /* specular */
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), object[objId].material.shininess);
    /* shadow and ambient occlusion */
    float shadow = (use_shadows == true ? softShadow(hit, lightDir, 0.1, 32) : 1.0);
    float mShadow = (use_shadows ? 1.0 - computeMeshShadows(hit, normal) : 1.0);
    float ao = (use_occlusion == true ? ambientOcclusion(hit, normal) : 1.0);
    /* compute terms */
    vec3 ambient  = directionalLight.ambient  * m_diffuse;
    vec3 diffuse  = directionalLight.diffuse  * diff * m_diffuse;
    vec3 specular = directionalLight.specular * spec * object[objId].material.specular;

    return ambient + (diffuse + specular) * mShadow * shadow * ao;
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

float   smoothBox( vec3 p, vec3 s, float r ) {
    return length(max(abs(p) - s, 0.0)) - r;
}

float sminCubic( float a, float b, float k ) {
    float h = max( k-abs(a-b), 0.0 );
    return min( a, b ) - h*h*h/(6.0*k*k);
}

float   blob( vec3 p ) {
    vec4 t = vec4(sin(uTime*0.8), cos(uTime*0.7), sin(uTime*1.6), cos(uTime*2.1));
    float s1 = sphere(p + 0.42 * vec3(t.w, t.y, t.x), 1.0);
    float s2 = sphere(p + 0.75 * vec3(t.z, t.x, t.y), 1.0);
    float s3 = sphere(p + 0.75 * vec3(t.y, t.w, t.z), 1.0);
    float s4 = sphere(p + 0.50 * vec3(t.x, t.z, t.w), 1.0);
    return sminCubic(sminCubic(sminCubic(s1, s2, 0.25), s3, 0.25), s4, 0.25);
}

vec2   mandelbulb(vec3 pos) {
	vec3 z = pos;
	float dr = 1.0;
	float r = 0.0;
    float t0 = 1.0;
	for (int i = 0; i < 4; ++i) {
		r = length(z);
		if (r > 1.5) break;
		float theta = acos(z.z/r)*8.0 + uTime * 0.1;
		float phi = atan(z.y,z.x)*8.0 + uTime * 0.05;
        float rp = pow(r, 7.0);
		dr =  rp*8.0*dr + 1.0;
		float zr = rp * r;
        float sinTheta = sin(theta);
		z = zr*vec3(sinTheta*cos(phi), sin(phi)*sinTheta, cos(theta)) + pos;
        t0 = min(t0, zr);
	}
	return vec2(0.25*log(r)*r/dr, t0);
}

/* mandelbox variables */
const float min_radius = 0.25;
const float scale = 2.0;
const int iters = 10;
const float minRadius2 = min_radius * min_radius;
const vec4 scalevec = vec4(scale, scale, scale, abs(scale)) / minRadius2;
const float C1 = abs(scale - 1.0), C2 = pow(abs(scale), float(1 - iters));

vec2   mandelbox( vec3 p ) {
    vec4 z = vec4(p.xyz * 6.0, 1.0), p0 = vec4(p.xyz * 6.0, 1.0);
    float t0 = 1.0;
    for (int i = 0; i < iters; i++) {
        z.xyz = clamp(z.xyz, -1.0, 1.0) * 2.0 - z.xyz;  // box fold
        float r2 = dot(z.xyz, z.xyz);
        z.xyzw *= clamp(max(minRadius2 / r2, minRadius2), 0.0, 1.0);  // sphere fold
        z.xyzw = z * scalevec + p0;
        t0 = min(t0, r2);
    }
	return vec2(((length(z.xyz) - C1) / z.w - C2) / 6.0, t0);
}

float   ifs( vec3 p ) {
    float ui = 100.0 * uTime * 0.1;
    float y = -0.001 * ui;
    mat2  m = mat2(sin(y), cos(y), -cos(y), sin(y));
    y = 0.0035 * ui;
    mat2  n = mat2(sin(y), cos(y), -cos(y), sin(y));
    y = 0.0023 * ui;
    mat2 nn = mat2(sin(y), cos(y), -cos(y), sin(y));

    float t = 1.0;
    for (int i = 0; i < 10; i++) {
        t = t * 0.66;
        p.xy =  m * p.xy;
        p.yz =  n * p.yz;
        p.zx = nn * p.zx;
        p.xz = abs(p.xz) - t;
    }
    return smoothBox(p, vec3(0.975 * t), 0.1 * t);
}
