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

uniform sampler2D depthBuffer;
uniform sObject object[MAX_OBJECTS];
uniform int nObjects;

uniform sDirectionalLight directionalLight;
uniform mat4 invProjection;
uniform mat4 invView;
uniform vec2 uMouse;
uniform float uTime;
uniform vec3 cameraPos;

const int 	maxRaySteps = 300;      // the maximum number of steps the raymarching algorithm is allowed to perform
const float maxDist = 5.0;          // the maximum distance the ray can travel in world-space
// const float minDist = 0.0005;       // the distance from object threshold at which we consider a hit in raymarching
const float minDist = 0.001;       // the distance from object threshold at which we consider a hit in raymarching

const int 	maxRayStepsShadow = 32; // the maximum number of steps the raymarching algorithm is allowed to perform for shadows
const float maxDistShadow = 3.0;    // the maximum distance the ray can travel in world-space
const float minDistShadow = 0.005;  // the distance from object threshold at which we consider a hit in raymarching for shadows


/* prototypes */
vec4    raymarch( in vec3 ro, in vec3 rd, float s );
vec3    getNormal( in vec3 p );
vec3    computeDirectionalLight( int objId, in vec3 hit, in vec3 normal, in vec3 viewDir, in vec3 m_diffuse, bool use_shadows );
float   softShadow( in vec3 ro, in vec3 rd, float mint, float k );
float   ambientOcclusion( in vec3 hit, in vec3 normal );
vec3    map( in vec3 p );

float   cube( vec3 p );
float   box( vec3 p, vec3 s );
float   torus( vec3 p );
vec2    mandelbulb( vec3 p );
vec2    mandelbox( vec3 p );


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
    vec4 res = raymarch(cameraPos, dir, depth);
    if (res.x == 0.0) { FragColor = vec4(0.0); return ; }

    int id = int(res.w);

    // compute useful variables for light
    vec3 hit = cameraPos + dir * res.x;
    vec3 normal = getNormal(hit);
    vec3 viewDir = normalize(cameraPos - hit);

    // compute colors
    // if (object[id].id == 0) { // mandelbox
    //     float fog = res.x * 0.5;
    //     float it = res.z / float(maxRaySteps);
    //     vec3 color = fog * fog * vec3(0.9, 0.517, 0.345);
    //     color += (it * vec3(log(it) * 0.231, it * 0.592, 0.776) + res.y * res.y * vec3(0.486, 0.125, 0.125)) * 0.25;
    //     vec3 light = computeDirectionalLight(id, hit, normal, viewDir, vec3(0.898, 0.325, 0.7231), false);
        // FragColor = vec4(light * color + color * 0.33, -log(it)*2.0);
    // }
    if (object[id].id == 0) { // mandelbox
        float fog = res.x * 0.5;
        float it = res.z / float(maxRaySteps);
        vec3 colorFog = fog * fog * vec3(0.9, 0.517, 0.345) * 0.5;
        vec3 color = (vec3(0.231, 0.592, 0.776) + res.y * res.y * vec3(0.486, 0.125, 0.125)) * 0.3;
        vec3 light = computeDirectionalLight(id, hit, normal, viewDir, vec3(0.898, 0.325, 0.7231), false);
        FragColor = vec4(light * color * vec3(0.9, 0.517, 0.345) * 1.2 + colorFog, -log(it)*2.0);
    }
    else if (object[id].id == 1) { // mandelbulb
        res.y = pow(clamp(res.y, 0.0, 1.0), 0.55);
        vec3 tc0 = 0.5 + 0.5 * sin(3.0 + 4.2 * res.y + vec3(0.0, 0.5, 1.0));
        vec3 diffuse = vec3(0.9, 0.8, 0.6) * 0.2 * tc0 * 8.0;
        vec3 color = computeDirectionalLight(id, hit, normal, viewDir, diffuse, true);
        FragColor = vec4(color, object[id].material.opacity);
    }
    else {
        vec3 color = computeDirectionalLight(id, hit, normal, viewDir, object[id].material.diffuse, true);
        FragColor = vec4(color, object[id].material.opacity);
    }

    // iterations color
    // {
    //     FragColor = vec4(res.z / float(maxRaySteps), 0, 0.03, 1.0);
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
            new = vec3(torus(pos), 0, 2);

        new.x *= object[i].scale;
        new.z = i;
        res = opU(res, new);
    }
    return res;
}

/* return: distance, t0, iteration, object_id */
vec4    raymarch( in vec3 ro, in vec3 rd, float s ) {
	float t = 0.0;
	for (int i = 0; i < maxRaySteps; i++) {
        vec3 res = map(ro + rd * t);
		t += res.x;
        if (t > maxDist || t > s) return vec4(0.0, 0.0, i, res.z); // optimization and geometry occlusion
		if (res.x < minDist) return vec4(t, res.y, i, res.z);
	}
	return vec4(0.0, 0.0, maxRaySteps, 0);
}

vec3    getNormal( in vec3 p ) {
    const vec2 eps = vec2(0.001, 0.0);
    return normalize(vec3(map(p + eps.xyy).x - map(p - eps.xyy).x,
                          map(p + eps.yxy).x - map(p - eps.yxy).x,
                          map(p + eps.yyx).x - map(p - eps.yyx).x));
}

vec3    computeDirectionalLight( int objId, in vec3 hit, in vec3 normal, in vec3 viewDir, in vec3 m_diffuse, bool use_shadows ) {
    vec3 lightDir = normalize(directionalLight.position);
    /* diffuse */
    float diff = max(dot(normal, lightDir), 0.0);
    /* specular */
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), object[objId].material.shininess);
    /* shadow */
    float shadow = (use_shadows == true ? softShadow(hit, lightDir, 0.1, 64) : 1.0);
    /* compute terms */
    vec3 ambient  = directionalLight.ambient  * m_diffuse;
    vec3 diffuse  = directionalLight.diffuse  * diff * m_diffuse * shadow;
    vec3 specular = directionalLight.specular * spec * object[objId].material.specular * shadow;
    return ambient + diffuse + specular;
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
        if (t > maxDistShadow) break; // check geometry occlusion here ?
	}
	return res;
}

/*  Distance Estimators
*/
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
    return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
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
        theta = asin(z.z / r);// + uTime * 0.01;
        phi = atan(z.y, z.x);// + uTime * 0.005;
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

    vec4 z = vec4(p.xyz, 1.0), p0 = vec4(p.xyz, 1.0);
    float t0 = 1.0;
    for (int i = 0; i < 12; i++) {
        z.xyz = clamp(z.xyz, -1.0, 1.0) * 2.0 - z.xyz;  // box fold
        float r2 = dot(z.xyz, z.xyz);
        z.xyzw *= clamp(max(minRadius2 / r2, minRadius2), 0.0, 1.0);  // sphere fold
        z.xyzw = z * scalevec + p0;
        t0 = min(t0, r2);
    }
	return vec2((length(z.xyz) - C1) / z.w - C2, t0);
}
