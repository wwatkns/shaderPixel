#version 400 core
out vec4 FragColor;

struct sObject {
    int         id;
    float       scale;
    float       boundingSphereScale;
    mat4        invMat;
};

in vec3 FragPos;
in vec2 TexCoords;
in float Near;
in float Far;

#define MAX_OBJECTS 8

uniform sampler2D depthMap;
uniform sObject object[MAX_OBJECTS];
uniform int nObjects;

uniform mat4 lightSpaceMat;

uniform mat4 invProjection;
uniform mat4 invView;

uniform vec2 uMouse;
uniform float uTime;
uniform vec3 cameraPos;

const int 	maxRaySteps = 128;      // the maximum number of steps the raymarching algorithm is allowed to perform
const float maxDist = 5000.0;         // the maximum distance the ray can travel in world-space
const float minDist = 0.001;        // the distance from object threshold at which we consider a hit in raymarching

/* prototypes */
vec4    raymarch( in vec3 ro, in vec3 rd, float s );
vec3    map( in vec3 p );
vec4    raymarchObj( in vec3 ro, in vec3 rd, float s, int i );
vec3    mapObj( in vec3 p, int i );

vec2    raySphere( in vec3 ro, in vec3 rd, in vec4 sph, float dbuffer );
float   sphere( vec3 p, float s );
float   cube( vec3 p );
float   box( vec3 p, vec3 s );
float   smoothBox( vec3 p, vec3 s, float r );
float   torus( vec3 p );
vec2    mandelbulb( vec3 p );
vec2    mandelbox( vec3 p );
float   ifs( vec3 p );


float random(vec2 p) {
    return fract(sin(mod(dot(p, vec2(12.9898,78.233)), 3.14))*43758.5453);
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

float   linearizeDepth( float depth ) {
    float z = depth * 2.0 - 1.0;
    return 2.0 * Near * Far / (Far + Near - z * (Far - Near));
}

void    main() {
    vec2 uv = vec2(TexCoords.x, 1.0 - TexCoords.y);
    vec3 ndc = vec3(uv * 2.0 - 1.0, -1.0);

    /* direction is converted from ndc to world-space */
    vec3 dir = (invProjection * vec4(ndc, 1.0)).xyz;
    dir = normalize((invView * vec4(dir, 0.0)).xyz);

    // THE DEPTH BUFFER HERE IS THE SHADOW DEPTH MAP FROM MESHES
    /* depth-buffer value conversion to world space (for correct distance for) */
    float depth = texture(depthMap, uv).x;
    depth = distance(cameraPos, worldPosFromDepth(depth, ndc));
    // float depth = linearizeDepth(texture(depthMap, uv).x);

    /* Bounding sphere optimisation */
    vec4 res = vec4(0.0);
    for (int i = 0; i < MAX_OBJECTS && i < nObjects; i++) {
        vec3 pos = (object[i].invMat * vec4(vec3(0.0), -1.0)).xyz;
        vec4 sphere = vec4(pos, object[i].scale * object[i].boundingSphereScale);
        vec2 bounds = raySphere(cameraPos, dir, sphere, depth);
        if (bounds.x < 0.0) { continue ; }
        
        if (object[i].id != 3 && object[i].id != 4) {
            vec3 ro = cameraPos + dir * bounds.x;
            vec4 tmp = raymarchObj(ro, dir, depth, i);

            if (tmp.x > 0.0) {
                tmp.x += bounds.x;
                if (tmp.x < res.x || res.x == 0.0)
                    res = tmp;
            }
        }
        else { /* volume SDF have simple sphere shadows (because it's too computationally expensive otherwise) */
            float tmp = min(bounds.x, depth);
            if (tmp > 0.0) {
                if (tmp < res.x || res.x == 0.0)
                    res = vec4(tmp);
            }
        }
    }

    if (res.x == 0.0)
        res.x = 100.;
    // FragColor = vec4(vec3(1.0-res.x*0.01), 1.0);
    FragColor = vec4(vec3(1.0-res.x*0.01, depth*0.01, depth*0.01), 1.0);

    // vec3 hit = cameraPos + dir * res.x;
    // vec4 Pclip = lightSpaceMat * vec4(hit, 1.);
    // float ndc_depth = Pclip.z / Pclip.w;
    // gl_FragDepth = (gl_DepthRange.diff * ndc_depth + gl_DepthRange.far + gl_DepthRange.near) / 2.0;
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
            new = vec3(mandelbox(pos), 0.);
        else if (object[i].id == 1)
            new = vec3(mandelbulb(pos), 1.);
        else if (object[i].id == 2)
            new = vec3(ifs(pos), 0., 2.);
        else if (object[i].id == 5)
            new = vec3(torus(pos), 0., 5.);
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
        res.x = torus(pos);
    res.x *= object[i].scale;
    return res;
}

vec4    raymarchObj( in vec3 ro, in vec3 rd, float s, int obj ) {
	float t = 0.0;
    ro += rd * minDist * random(gl_FragCoord.xy/256.);
	for (int i = 0; i < maxRaySteps; i++) {
        vec3 res = mapObj(ro + rd * t, obj);
		t += res.x;
        if (t > maxDist || t > s) return vec4(0.0, 0.0, i, 0.0); // optimization and geometry occlusion
		if (res.x < minDist) return vec4(t, res.y, i, res.z);
	}
	return vec4(0.0, 0.0, maxRaySteps, 0.0);
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

vec2   mandelbulb(vec3 pos) {
	vec3 z = pos;
	float dr = 1.0;
	float r = 0.0;
    float t0 = 1.0;
	for (int i = 0; i < 4; i++) {
		r = length(z);
		if (r > 2.0) break;
		// convert to polar coordinates
		float theta = acos(z.z/r) + uTime * 0.1;
		float phi = atan(z.y,z.x) + uTime * 0.05;
		dr =  pow(r, 7.0)*8.0*dr + 1.0;
		// scale and rotate the point
		float zr = pow(r, 8.0);
		theta = theta*8.0;
		phi = phi*8.0;
		// convert back to cartesian coordinates
		z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta)) + pos;
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
    for (int i = 0; i < 12; i++) {
        t = t * 0.66;
        p.xy =  m * p.xy;
        p.yz =  n * p.yz;
        p.zx = nn * p.zx;
        p.xz = abs(p.xz) - t;
    }
    return smoothBox(p, vec3(0.975 * t), 0.1 * t);
}
