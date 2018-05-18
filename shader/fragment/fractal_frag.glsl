#version 400 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in mat4 invProj;
in mat4 invView;
in float Near;
in float Far;

uniform sampler2D depthBuffer;
uniform vec2 uResolution;
uniform vec2 uMouse;
uniform float uTime;
uniform vec3 uCameraPos;

int 	MaximumRaySteps = 128;   // the maximum number of steps the raymarching algorithm can perform
float 	MaximumDistance = 32;    // the maximum distance for the raymarching ray
float 	MinimumDistance = 0.001; // the minimum distance to render the pixel

int		Iterations = 12;         // the number of iterations for the fractals

/* prototypes */
vec2    raymarch( vec3 origin, vec3 dir, float s );
vec3    getNormal( vec3 p );
vec3    computeDirectionalLight( vec3 normal, vec3 viewDir, vec3 m_diffuse );
float   cube( vec3 p );
float   torus( vec3 p );
float   tetraHedron( vec3 p );
vec2    mandelbulb( vec3 p );


void    main() {
    vec2 uv = vec2(TexCoords.x, 1.0 - TexCoords.y);
    vec3 ndc = vec3(uv * 2.0 - 1.0, -0.1);

    vec3 origin = uCameraPos;
    // direction is converted from ndc to world-space
    vec3 dir = (invProj * vec4(ndc, 1.0)).xyz;
    dir = normalize(vec3(invView * vec4(dir, 0.0)));

    // convert depth buffer value to world depth
    float depth = texture(depthBuffer, uv).x * 2.0 - 1.0;
    depth = 2.0 * Near * Far / (Far + Near - depth * (Far - Near));

    vec2 res = raymarch(origin, dir, depth);

    // colorize
    res.y = pow(clamp(res.y, 0.0, 1.0), 0.55);
    vec3 tc0 = 0.5 + 0.5 * sin(3.0 + 4.2 * res.y + vec3(0.0, 0.5, 1.0));
    vec3 color = vec3(0.9, 0.8, 0.6) * 0.2 * tc0;

    vec3 normal = getNormal(origin + res.x * dir);
    vec3 viewDir = normalize(uCameraPos - FragPos);
    vec3 light = computeDirectionalLight(normal, viewDir, color);

    FragColor = vec4(light, 1.0);
    if (res.x == 0.0) FragColor.w = 0.0;

    /* DEBUG: display depth buffer */
    // float near = 0.1;
    // float far = 100.0;
    // float depth = texture(depthBuffer, uv).x;
    // float c = (2.0 * near) / (far + near - depth * (far - near));
    // FragColor = vec4(c, c, c, 1.0);
}

vec2    raymarch( vec3 origin, vec3 dir, float s ) {
	float totalDistance = 0.0;
	int steps;
	for (steps = 0; steps < MaximumRaySteps; ++steps) {
		vec3 p = origin + totalDistance * dir;
        vec2 res = mandelbulb(p);
        float distance = res.x;
		totalDistance += distance;
        /* optimization and geometry occlusion */
        if (totalDistance > MaximumDistance || totalDistance > s) return vec2(0.0);
		if (distance < MinimumDistance) return vec2(totalDistance, res.y);
	}
	return vec2(0.0);
}


vec3    getNormal( vec3 p ) {
    vec2 eps = vec2(0.001, 0.0);
    return normalize(vec3(mandelbulb(p + eps.xyy).x - mandelbulb(p - eps.xyy).x,
                          mandelbulb(p + eps.yxy).x - mandelbulb(p - eps.yxy).x,
                          mandelbulb(p + eps.yyx).x - mandelbulb(p - eps.yyx).x));
}

vec3    computeDirectionalLight( vec3 normal, vec3 viewDir, vec3 m_diffuse ) {
    vec3 l_ambient = vec3(0.77, 0.88, 1.0) * 0.05;
    vec3 l_diffuse = vec3(1.0, 0.964, 0.77);
    vec3 l_specular = vec3(1.0, 1.0, 1.0);

    m_diffuse *= 8.0;
    vec3 m_specular = vec3(0.35, 0.35, 0.35);
    float m_shininess = 100.0;

    vec3 lightDir = normalize(vec3(10, 10, 6));
    /* diffuse */
    float diff = max(dot(normal, lightDir), 0.0);
    /* specular */
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), m_shininess);

    /* compute terms */
    vec3 ambient  = l_ambient  * m_diffuse;
    vec3 diffuse  = l_diffuse * diff * m_diffuse;
    vec3 specular = l_specular * spec * m_specular;
    return ambient + diffuse + specular;
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
    while (n < Iterations) {
       if (p.x + p.y < 0.0) p.xy = -p.yx; // fold 1
       if (p.x + p.z < 0.0) p.xz = -p.zx; // fold 2
       if (p.y + p.z < 0.0) p.zy = -p.yz; // fold 3
       p = p * 2.0 - 1.0 * (2.0 - 1.0);
       n++;
    }
    return length(p) * pow(2.0, -float(n));
}

vec2   mandelbulb( vec3 p ) {
    float power = 4.0;
	vec3 z = p;
	float dr = 1.0;
	float r, theta, phi;
    float t0 = 1.0;
	for (int i = 0; i < Iterations ; ++i) {
		r = length(z);
		if (r > 2.0) break;
		// convert to polar coordinates
		theta = asin(z.z / r) + uTime * 0.1;
		phi = atan(z.y, z.x);
		dr = pow(r, power - 1.0) * power * dr + 1.0;
		// scale and rotate the point
		r = pow(r, power);
		theta = theta * power;
		phi = phi * power;
		// convert back to cartesian coordinates
		z = r * vec3(cos(theta) * cos(phi), sin(phi) * cos(theta), sin(theta)) + p;
        t0 = min(t0, r);
	}
	return vec2(0.5 * log(r) * r / dr, t0);
}
