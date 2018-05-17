#version 400 core
out vec4 FragColor;

in vec3 FragPos;   // FragPos is relative to world
in vec2 TexCoords; // TexCoords is relative to surface
in mat4 Proj;

uniform vec2 uResolution;
uniform vec2 uMouse;
uniform float uTime;

uniform mat4 uView;

int 	MaximumRaySteps = 50;
float 	MinimumDistance = 0.005;
int		Iterations = 10;

/* prototypes */
float   trace( vec3 origin, vec3 dir );
float   tetraHedron( vec3 p );

void main() {
    vec2 uv = TexCoords.xy / uResolution.xy * 3.0 - 3.0;

    vec3 origin = vec3(uv, 1.0);
    vec3 dir = mat3(Proj) * mat3(uView) * normalize(vec3(cos(uTime*0.5), sin(uTime), 2.0));
    // vec3 dir = normalize(vec3(cos(uTime*0.5), sin(uTime), 1.0));

    // vec3 origin = mat3(ViewProj) * vec3(uv, -1.0);
    // vec3 dir = normalize(mat3(ViewProj) * vec3(0.0, 0.0, 1.0));
    // vec3 dir = normalize(uCameraPos);

    float col = trace(origin, dir);

    FragColor = vec4(col*col, col, col, 1.0);
    if (col == 0.0) FragColor.w = 0.0; // replace black background by transparency
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

float   trace( vec3 origin, vec3 dir ) {
	float totalDistance = 0.0;
	int steps;
	for (steps = 0; steps < MaximumRaySteps; ++steps) {
		vec3 p = origin + totalDistance * dir;
        float distance = tetraHedron(p);
		totalDistance += distance;
		if (distance < MinimumDistance) break;
	}
	return 1.0 - float(steps) / float(MaximumRaySteps);
}
