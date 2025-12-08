#version 330 core

struct Light {
    int type; // 0=directional, 1=point, 2=spot
    vec3 position;
    vec3 direction;
    vec3 color;
    vec3 function;
    float angle;
    float penumbra;
};

struct Material {
    vec4 cAmbient;
    vec4 cDiffuse;
    vec4 cSpecular;
    float shininess;
};

in vec3 worldPos;
in vec3 worldNormal;
in vec2 fragUV;
in mat3 TBN;

out vec4 color;

uniform int numLights;
uniform Light lights[8];
uniform vec3 cameraPos;
uniform Material material;

uniform float k_a;
uniform float k_d;
uniform float k_s;

// Textures
uniform sampler2D diffuseTexture;  // Add this uniform
uniform sampler2D normalMap;
uniform bool useNormalMap;
uniform sampler2D bumpMap;
uniform bool useBumpMap;
uniform float bumpStrength;

vec3 computeBumpNormal(sampler2D heightMap, vec2 uv, float strength) {
    vec2 texSize = textureSize(heightMap, 0);
    vec2 texelSize = 1.0 / texSize;

    float h_center = texture(heightMap, uv).r;
    float h_right = texture(heightMap, uv + vec2(texelSize.x, 0.0)).r;
    float h_top = texture(heightMap, uv + vec2(0.0, texelSize.y)).r;

    float dh_dx = (h_right - h_center) * strength;
    float dh_dy = (h_top - h_center) * strength;

    return normalize(vec3(-dh_dx, -dh_dy, 1.0));
}

vec3 computeLightContribution(vec3 N, vec3 V, vec3 worldPos, Light light) {
    vec3 L;
    float attenuation = 1.0;

    if (light.type == 0) {
        // Directional light
        L = normalize(-light.direction);
    } else {
        // Point or Spot light
        vec3 toLight = light.position - worldPos;
        float dist = length(toLight);
        L = normalize(toLight);

        float c = light.function.x;
        float lin = light.function.y;
        float q = light.function.z;

        float denom = c + lin * dist + q * dist * dist;
        if (abs(denom) > 1.0)
            attenuation = 1.0 / denom;

        if (light.type == 2) {
            // Spot light
            float theta = acos(dot(normalize(-light.direction), normalize(L)));
            float inner = light.angle - light.penumbra;
            float outer = light.angle;
            float intensity = 0.0;

            if (theta < inner) {
                intensity = 1.0;
            } else if (theta > outer) {
                intensity = 0.0;
            } else {
                float ratio = (theta - inner) / (outer - inner);
                float falloff = -2.0 * pow(ratio, 3.0) + 3.0 * pow(ratio, 2.0);
                intensity = 1.0 - falloff;
            }
            attenuation *= clamp(intensity, 0.0, 1.0);
        }
    }

    float NdotL = max(dot(N, L), 0.0);
    if (NdotL <= 0.0) return vec3(0.0);

    vec3 R = reflect(-L, N);
    float RdotV = max(dot(R, V), 0.0);
    float spec = max(pow(RdotV, material.shininess), 0.0);

    vec3 diffuse = material.cDiffuse.rgb * k_d * NdotL * light.color;
    vec3 specular = material.cSpecular.rgb * k_s * spec * light.color;

    return attenuation * (diffuse + specular);
}

void main() {
    // Sample diffuse texture
    vec4 texColor = texture(diffuseTexture, fragUV);

    // Compute normal based on mapping techniques
    vec3 N;

    if (useBumpMap && useNormalMap) {
        // Hybrid mode: combine bump and normal mapping
        vec3 bumpNormal = computeBumpNormal(bumpMap, fragUV, bumpStrength);
        vec3 normalMapNormal = texture(normalMap, fragUV).rgb * 2.0 - 1.0;
        vec3 combinedNormal = normalize(bumpNormal * 0.5 + normalMapNormal * 0.5);
        N = normalize(TBN * combinedNormal);
    } else if (useBumpMap) {
        // Bump mapping only
        vec3 tangentSpaceNormal = computeBumpNormal(bumpMap, fragUV, bumpStrength);
        N = normalize(TBN * tangentSpaceNormal);
    } else if (useNormalMap) {
        // Normal mapping only
        vec3 tangentSpaceNormal = texture(normalMap, fragUV).rgb * 2.0 - 1.0;
        N = normalize(TBN * tangentSpaceNormal);
    } else {
        // No mapping - use interpolated normal
        N = normalize(worldNormal);
    }

    vec3 V = normalize(cameraPos - worldPos);

    // Ambient component - multiply by texture
    vec3 total = material.cAmbient.rgb * k_a * texColor.rgb;

    // Add contribution from each light - multiply diffuse by texture
    for (int i = 0; i < numLights; ++i) {
        total += computeLightContribution(N, V, worldPos, lights[i]) * texColor.rgb;
    }

    color = vec4(total, texColor.a);
}
