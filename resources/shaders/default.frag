#version 330 core

in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragUV;
in mat3 TBN;

out vec4 fragColor;

uniform vec4 materialAmbient;
uniform vec4 materialDiffuse;
uniform vec4 materialSpecular;
uniform float materialShininess;

uniform float ka;
uniform float kd;
uniform float ks;

uniform vec3 cameraPos;

// texture
uniform sampler2D colorTexture;
uniform bool useColorTexture;
uniform sampler2D normalMap;
uniform bool useNormalMap;
uniform sampler2D bumpMap;
uniform bool useBumpMap;
uniform float bumpStrength;

struct Light {
    int type;
    vec4 color;
    vec3 function;
    vec4 pos;
    vec4 dir;
    float penumbra;
    float angle;
};

uniform Light lights[8];
uniform int numLights;


vec3 computeBumpNormal(sampler2D heightMap, vec2 uv, float strength) {

    vec2 texSize = textureSize(heightMap, 0);
    vec2 texelSize = 1.0 / texSize;

    float h_center = texture(heightMap, uv).r;
    float h_right = texture(heightMap, uv + vec2(texelSize.x, 0.0)).r;
    float h_top = texture(heightMap, uv + vec2(0.0, texelSize.y)).r;

    float dh_dx = (h_right - h_center) * strength;
    float dh_dy = (h_top - h_center) * strength;

    vec3 normal = normalize(vec3(-dh_dx, -dh_dy, 1.0));

    return normal;
}

vec3 calculateLighting(Light light, vec3 position, vec3 normal, vec3 viewDir, vec3 materialDiff) {
    vec3 lightDir;
    float attenuation = 1.0;

    if (light.type == 0) {

        lightDir = normalize(-vec3(light.dir));
    } else if (light.type == 1 || light.type == 2) {

        vec3 lightToPos = vec3(light.pos) - position;
        float distance = length(lightToPos);
        lightDir = normalize(lightToPos);

        float a = light.function.x;
        float b = light.function.y;
        float c = light.function.z;
        attenuation = min(1.0, 1.0 / (a + b * distance + c * distance * distance));

        if (light.type == 2) {

            vec3 spotDir = normalize(vec3(light.dir));
            float cosX = dot(-lightDir, spotDir);
            float x = acos(clamp(cosX, -1.0, 1.0));

            float outerAngle = light.angle;
            float innerAngle = outerAngle - light.penumbra;

            if (x > outerAngle) {
                return vec3(0.0);
            }

            float intensity = 1.0;
            if (x > innerAngle) {
                float t = (x - innerAngle) / (outerAngle - innerAngle);
                float falloff = -2.0 * t * t * t + 3.0 * t * t;
                intensity = 1.0 - falloff;
            }

            attenuation *= intensity;
        }
    }

    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuseComponent = kd * diff * materialDiff * vec3(light.color);

    // specular
    vec3 directionFromLight = -lightDir;
    vec3 reflectDir = normalize(directionFromLight - 2.0 * dot(normal, directionFromLight) * normal);
    float spec = pow(max(dot(reflectDir, viewDir), 0.0), materialShininess);

    vec3 specularComponent = ks * spec * vec3(materialSpecular) * vec3(light.color);

    return attenuation * (diffuseComponent + specularComponent);
}

void main() {

    vec2 tiledUV = fragUV * vec2(2.0, 1.0);

    vec3 baseColor;
    if (useColorTexture) {
        baseColor = texture(colorTexture, tiledUV).rgb;
    } else {
        baseColor = vec3(materialDiffuse);
    }

    vec3 normal;

    if (useBumpMap && useNormalMap) {

        vec3 bumpNormal = computeBumpNormal(bumpMap, tiledUV, bumpStrength);

        vec3 normalMapNormal = texture(normalMap, tiledUV).rgb * 2.0 - 1.0;

        vec3 combinedNormal = normalize(bumpNormal * 0.5 + normalMapNormal * 0.5);

        normal = normalize(TBN * combinedNormal);

    } else if (useBumpMap) {

        vec3 tangentSpaceNormal = computeBumpNormal(bumpMap, tiledUV, bumpStrength);

        normal = normalize(TBN * tangentSpaceNormal);
    } else if (useNormalMap) {

        vec3 tangentSpaceNormal = texture(normalMap, tiledUV).rgb;

        tangentSpaceNormal = tangentSpaceNormal * 2.0 - 1.0;

        normal = normalize(TBN * tangentSpaceNormal);
    } else {
        normal = normalize(fragNormal);
    }

    vec3 viewDir = normalize(cameraPos - fragPosition);

    vec3 ambientComponent = ka * vec3(materialAmbient) * baseColor;
    vec3 totalLighting = ambientComponent;

    for (int i = 0; i < numLights && i < 8; i++) {
        totalLighting += calculateLighting(lights[i], fragPosition, normal, viewDir, baseColor);
    }

    totalLighting = clamp(totalLighting, 0.0, 1.0);
    fragColor = vec4(totalLighting, 1.0);
}
