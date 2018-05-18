#version 400 core
out vec4 FragColor;

in vec3 FragPos;   // FragPos is relative to world
in vec2 TexCoords; // TexCoords is relative to surface
in mat4 invProj;
in mat4 invView;

uniform vec2 uResolution;
uniform vec2 uMouse;
uniform float uTime;
uniform vec3 uCameraPos;

int 	MaximumRaySteps = 100;   // the maximum number of steps the raymarching algorithm can perform
float 	MaximumDistance = 20;    // the maximum distance for the raymarching ray
float 	MinimumDistance = 0.005; // the minimum distance to render the pixel

int		Iterations = 10;         // the number of iterations for the fractals

/* prototypes */
float   raymarch( vec3 origin, vec3 dir );
float   tetraHedron( vec3 p );
float   cube( vec3 p );
vec3    getNormal( vec3 p );

void main() {
    vec2 uv = TexCoords.xy;// / uResolution.xy * 3.0;// - 3.0;
    vec3 ndc = vec3(uv * 2.0 - 1.0, -0.1); // -1.0
    ndc.y = -ndc.y;

    vec3 origin = uCameraPos;
    /* direction is converted from ndc to world-space */
    vec3 dir = (invProj * vec4(ndc, 1.0)).xyz;
    dir = normalize(vec3(invView * vec4(dir, 0.0)));

    float col = raymarch(origin, dir);

    FragColor = vec4(col, col, col, 1.0);
    if (col == 0.0) FragColor.w = 0.0; // replace black background by transparency
}

float   cube( vec3 p ) {
    vec3 d = abs(p) - vec3(1, 1, 1);
    return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

float   tetraHedron( vec3 p ) {
    float r;
    int n = 0;
    while (n < Iterations) {
       if (p.x + p.y < 0.0) p.xy = -p.yx; // fold 1
       if (p.x + p.z < 0.0) p.xz = -p.zx; // fold 2
       if (p.y + p.z < 0.0) p.zy = -p.yz; // fold 3
       p = p * 2.0 - 1.0 * (2.0 - 1.0);
       n++;
    }
    return length(p) * pow(2.0, -float(n));
}

float   raymarch( vec3 origin, vec3 dir ) {
	float totalDistance = 0.0;
	int steps;
    // vec3 n;
	for (steps = 0; steps < MaximumRaySteps; ++steps) {
		vec3 p = origin + totalDistance * dir;
        // n = p; // NEW
        float distance = tetraHedron(p);
		totalDistance += distance;
        if (totalDistance > MaximumDistance) return 0.0; // optimization
		if (distance < MinimumDistance) break;
	}
    // n = getNormal(n);
    // return dot(normalize(vec3(-1, -1, 2)), n);
	return 1.0 - float(steps) / float(MaximumRaySteps);
}

vec3    getNormal( vec3 p ) {
    vec2 eps = vec2(0.001, 0.0);
    return normalize(vec3(tetraHedron(p + eps.xyy) - tetraHedron(p - eps.xyy),
                          tetraHedron(p + eps.yxy) - tetraHedron(p - eps.yxy),
                          tetraHedron(p + eps.yyx) - tetraHedron(p - eps.yyx)));
}
