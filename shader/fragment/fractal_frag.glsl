#version 400 core
out vec4 FragColor;

struct sDirectionalLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct sMaterial {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    float opacity;
};

in vec3 FragPos;
in vec2 TexCoords;
in float Near;
in float Far;

uniform sampler2D depthBuffer;
uniform sMaterial material;
uniform sDirectionalLight directionalLight;
uniform mat4 invProjection;
uniform mat4 invView;
uniform vec2 uMouse;
uniform float uTime;
uniform vec3 cameraPos;

const int 	maxRaySteps = 128;      // the maximum number of steps the raymarching algorithm is allowed to perform
const float maxDist = 5.0;          // the maximum distance the ray can travel in world-space
const float minDist = 0.0005;       // the distance from object threshold at which we consider a hit in raymarching

const int 	maxRayStepsShadow = 32; // the maximum number of steps the raymarching algorithm is allowed to perform for shadows
const float maxDistShadow = 3.0;    // the maximum distance the ray can travel in world-space
const float minDistShadow = 0.005;  // the distance from object threshold at which we consider a hit in raymarching for shadows

const int	maxIterations = 6;      // the number of iterations for the fractals


/* prototypes */
vec2    raymarch( in vec3 ro, in vec3 rd, float s );
vec3    getNormal( in vec3 p );
vec3    computeDirectionalLight( sDirectionalLight light, in vec3 hit, in vec3 normal, in vec3 viewDir, in vec3 m_diffuse );
float   softShadow( in vec3 ro, in vec3 rd, float mint, float k );
float   ambientOcclusion( in vec3 hit, in vec3 normal );
float   cube( vec3 p );
float   torus( vec3 p );
float   tetraHedron( vec3 p );
vec2    mandelbulb( vec3 p );


void    main() {
    vec2 uv = vec2(TexCoords.x, 1.0 - TexCoords.y);
    vec3 ndc = vec3(uv * 2.0 - 1.0, -1.0);

    // direction is converted from ndc to world-space
    vec3 dir = (invProjection * vec4(ndc, 1.0)).xyz;
    dir = normalize((invView * vec4(dir, 0.0)).xyz);

    // convert depth buffer value to world depth
    float depth = texture(depthBuffer, uv).x * 2.0 - 1.0;
    depth = 2.0 * Near * Far / (Far + Near - depth * (Far - Near));

    // visualize iteration count
	// float t = 0.0;
	// for (int i = 0; i < maxRaySteps; i++) {
    //     vec2 res = mandelbulb(cameraPos + dir * t);
	// 	t += res.x;
    //     float it = i / float(maxRaySteps);
    //     // optimization and geometry occlusion
    //     if (t > maxDist || t > depth) { FragColor = vec4(it, 0, 0.03, 1.0); break; }
	// 	if (res.x < minDist) { FragColor = vec4(it, 0, 0.03, 1.0); break; }
	// }
	// return ;

    vec2 res = raymarch(cameraPos, dir, depth);

    // early return if raymarch did not collide
    if (res.x == 0.0) {
        FragColor = vec4(0.0);
        return ;
    }
    // colorize
    res.y = pow(clamp(res.y, 0.0, 1.0), 0.55);
    vec3 tc0 = 0.5 + 0.5 * sin(3.0 + 4.2 * res.y + vec3(0.0, 0.5, 1.0));
    vec3 diffuse = vec3(0.9, 0.8, 0.6) * 0.2 * tc0 * 8.0;

    vec3 hit = cameraPos + dir * res.x;
    vec3 normal = getNormal(hit);
    vec3 viewDir = normalize(cameraPos - hit);
    vec3 color = computeDirectionalLight(directionalLight, hit, normal, viewDir, diffuse);

    FragColor = vec4(color, material.opacity);

    /* DEBUG: display depth buffer */
    // float near = 0.1;
    // float far = 100.0;
    // float depth = texture(depthBuffer, uv).x;
    // float c = (2.0 * near) / (far + near - depth * (far - near));
    // FragColor = vec4(c, c, c, 1.0);
}

vec2    raymarch( in vec3 ro, in vec3 rd, float s ) {
	float t = 0.0;
	for (int i = 0; i < maxRaySteps; i++) {
        vec2 res = mandelbulb(ro + rd * t);
		t += res.x;
        // optimization and geometry occlusion
        if (t > maxDist || t > s) return vec2(0.0);
		if (res.x < minDist) return vec2(t, res.y);
	}
	return vec2(0.0);
}

vec3    getNormal( in vec3 p ) {
    const vec2 eps = vec2(minDist, 0.0);
    return normalize(vec3(mandelbulb(p + eps.xyy).x - mandelbulb(p - eps.xyy).x,
                          mandelbulb(p + eps.yxy).x - mandelbulb(p - eps.yxy).x,
                          mandelbulb(p + eps.yyx).x - mandelbulb(p - eps.yyx).x));
}

vec3    computeDirectionalLight( sDirectionalLight light, in vec3 hit, in vec3 normal, in vec3 viewDir, in vec3 m_diffuse ) {
    vec3 lightDir = normalize(light.position);
    /* diffuse */
    float diff = max(dot(normal, lightDir), 0.0);
    /* specular */
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    /* shadow */
    float shadow = softShadow(hit, lightDir, 0.1, 64);
    /* compute terms */
    vec3 ambient  = light.ambient  * m_diffuse;
    vec3 diffuse  = light.diffuse  * diff * m_diffuse * shadow;
    vec3 specular = light.specular * spec * material.specular * shadow;
    return ambient + diffuse + specular;
}

float   softShadow( in vec3 ro, in vec3 rd, float mint, float k ) {
    float t = mint;
    float res = 1.0;
	for (int i = 0; i < maxRayStepsShadow; ++i) {
        float h = mandelbulb(ro + rd * t).x;
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

float   tetraHedron( vec3 p ) {
    float r;
    int n = 0;
    while (n < maxIterations) {
       if (p.x + p.y < 0.0) p.xy = -p.yx; // fold 1
       if (p.x + p.z < 0.0) p.xz = -p.zx; // fold 2
       if (p.y + p.z < 0.0) p.zy = -p.yz; // fold 3
       p = p * 2.0 - 1.0 * (2.0 - 1.0);
       n++;
    }
    return length(p) * pow(2.0, -float(n));
}

vec2   mandelbulb( vec3 p ) {
    const float power = 8.0;
    vec3 z = p;
    float dr = 1.0;
    float r, theta, phi;
    float t0 = 1.0;
    for (int i = 0; i < maxIterations; i++) {
        r = length(z);
        if (r > 2.0) break;
        // convert to polar coordinates
        theta = asin(z.z / r);// + uTime * 0.1;
        phi = atan(z.y, z.x);// + uTime * 0.1;
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
	return vec2(0.5 * log(r) * r / dr, t0);
}
