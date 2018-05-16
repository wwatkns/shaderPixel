#version 400 core
out vec4 FragColor;

// the direction is always from the position to the center of the scene
struct sDirectionalLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct sPointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float quadratic;
    float constant;
    float linear;
};

struct sMaterial {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    float opacity;
};

struct sState {
    bool use_shadows;
    bool use_texture_diffuse;
    bool use_texture_normal;
};


in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 Tangent;
in vec3 Bitangent;
in vec4 FragPosLightSpace;

#define MAX_POINT_LIGHTS 8


uniform sampler2D shadowMap;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;

uniform vec3 viewPos;
uniform sMaterial material;
uniform sDirectionalLight directionalLight;
uniform sPointLight pointLights[MAX_POINT_LIGHTS];
uniform int nPointLights;
uniform sState state;


/* prototypes */
vec3 computeDirectionalLight( sDirectionalLight light, vec3 normal, vec3 viewDir, vec4 fragPosLightSpace );
vec3 computePointLight( sPointLight light, vec3 normal, vec3 fragPos,vec3 viewDir );
float computeShadows( vec4 fragPosLightSpace, sDirectionalLight light );
vec3 computeNormal( void );


void main() {
    vec3 normal = computeNormal();
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = computeDirectionalLight(directionalLight, normal, viewDir, FragPosLightSpace);
    for (int i = 0; i < nPointLights && i < MAX_POINT_LIGHTS; ++i)
        result += computePointLight(pointLights[i], normal, FragPos, viewDir);

    FragColor = (state.use_texture_diffuse ? texture(texture_diffuse1, TexCoords) * vec4(result, 1.0f) : vec4(result, 1.0f));
    FragColor.w = material.opacity;
}

vec3 computeNormal( void ) {
    if (state.use_texture_normal) {
        mat3 TBN = mat3(Tangent, cross(Tangent, Normal), Normal);
        return (TBN * normalize(texture(texture_normal1, TexCoords).rgb * 2.0 - 1.0));
    }
    return (normalize(Normal));
}

vec3 computeDirectionalLight( sDirectionalLight light, vec3 normal, vec3 viewDir, vec4 fragPosLightSpace ) {
    vec3 lightDir = normalize(-(vec3(0, 0, 0) - light.position));
    /* diffuse */
    float diff = max(dot(normal, lightDir), 0.0);
    /* specular */
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    /* compute terms */
    vec3 ambient  = light.ambient  * material.diffuse;
    vec3 diffuse  = light.diffuse  * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    /* for diffuse and specular textures */
    // vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
    // vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
    // vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    if (state.use_shadows) {
        float shadow  = computeShadows(fragPosLightSpace, light);
        return (ambient + (1.0 - shadow) * (diffuse + specular));
    }
    return (ambient + (diffuse + specular));
}

vec3 computePointLight( sPointLight light, vec3 normal, vec3 fragPos, vec3 viewDir ) {
    vec3 lightDir = normalize(light.position - fragPos);
    /* diffuse */
    float diff = max(dot(normal, lightDir), 0.0);
    /* specular */
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    /* attenuation */
    float dist = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));
    /* compute terms */
    vec3 ambient  = light.ambient  * material.diffuse * attenuation;
    vec3 diffuse  = light.diffuse  * diff * material.diffuse * attenuation;
    vec3 specular = light.specular * spec * material.specular * attenuation;
    /* for diffuse and specular textures */
    // vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords)) * attenuation;
    // vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords)) * attenuation;
    // vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords)) * attenuation;
    return (ambient + diffuse + specular);
}

float computeShadows( vec4 fragPosLightSpace, sDirectionalLight light ) {
    /* perform perspective divide and put in interval [0,1] */
    vec3 projCoords = (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float bias = 0.0025;
    /* Default */
    // float shadow = (currentDepth - bias > closestDepth ? 1.0 : 0.0);
    /* PCF */
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias > pcfDepth ? 1.0 : 0.0);
        }
    }
    shadow /= 9.0;
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (projCoords.z > 1.0)
        shadow = 0.0;
    return (shadow);
}
