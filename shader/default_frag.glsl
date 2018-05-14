#version 400 core
out vec4 FragColor;

struct sDirectionalLight {
    vec3 direction;
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

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

#define MAX_POINT_LIGHTS 8

// uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap;

uniform vec3 viewPos;
uniform sMaterial material;
uniform sDirectionalLight directionalLight;
uniform sPointLight pointLights[MAX_POINT_LIGHTS];
uniform int nPointLights;

/* prototypes */
vec3 computeDirectionalLight( sDirectionalLight light, vec3 normal, vec3 viewDir, vec4 fragPosLightSpace );
vec3 computePointLight( sPointLight light, vec3 normal, vec3 fragPos,vec3 viewDir );
float computeShadows( vec4 fragPosLightSpace, sDirectionalLight light );


void main() {
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = computeDirectionalLight(directionalLight, normal, viewDir, FragPosLightSpace);
    // for (int i = 0; i < nPointLights && i < MAX_POINT_LIGHTS; ++i)
        // result += computePointLight(pointLights[i], normal, FragPos, viewDir);

    // FragColor = texture(texture_diffuse1, TexCoords) + vec4(result, 1.0f);
    FragColor = vec4(result, 1.0f);
    FragColor.w = material.opacity;
}

vec3 computeDirectionalLight( sDirectionalLight light, vec3 normal, vec3 viewDir, vec4 fragPosLightSpace ) {
    vec3 lightDir = normalize(-light.direction);
    /* diffuse */
    float diff = max(dot(normal, lightDir), 0.0);
    /* specular */
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    /* compute terms */
    vec3 ambient  = light.ambient  * material.diffuse;
    vec3 diffuse  = light.diffuse  * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    float shadow  = computeShadows(fragPosLightSpace, light);
    /* for diffuse and specular textures */
    // vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
    // vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
    // vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));

    // return (ambient + (diffuse + specular));
    return (ambient + (1.0 - shadow) * (diffuse + specular));
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
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(Normal);

    // vec3 lightDir = normalize(lightPos - FragPos);
    vec3 lightDir = normalize(vec3(1, 10, -1) - FragPos);

    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = (currentDepth - bias > closestDepth ? 1.0 : 0.0);
    // PCF
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
