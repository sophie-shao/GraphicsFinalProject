#version 330 core

in vec3 worldPos;
in vec3 worldNormal;
in vec2 fragUV;
in mat3 TBN;

out vec4 color;

uniform float k_a;
uniform float k_d;
uniform float k_s;
uniform float shininess;

uniform vec3 lightPos;
uniform vec3 cameraPos;

// texture
uniform sampler2D colorTexture;
uniform bool useColorTexture;
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

    vec3 normal = normalize(vec3(-dh_dx, -dh_dy, 1.0));

    return normal;
}

void main() {

    vec2 tiledUV = fragUV * vec2(2.0, 1.0);

    vec3 baseColor;
    if (useColorTexture) {
        baseColor = texture(colorTexture, tiledUV).rgb;
    } else {
        baseColor = vec3(1.0);
    }

    vec3 N;

    if (useBumpMap && useNormalMap) {
        vec3 bumpNormal = computeBumpNormal(bumpMap, tiledUV, bumpStrength);
        vec3 normalMapNormal = texture(normalMap, tiledUV).rgb * 2.0 - 1.0;
        vec3 combinedNormal = normalize(bumpNormal * 0.5 + normalMapNormal * 0.5);
        N = normalize(TBN * combinedNormal);
    } else if (useBumpMap) {
        vec3 tangentSpaceNormal = computeBumpNormal(bumpMap, tiledUV, bumpStrength);
        N = normalize(TBN * tangentSpaceNormal);
    } else if (useNormalMap) {
        vec3 tangentSpaceNormal = texture(normalMap, tiledUV).rgb * 2.0 - 1.0;
        N = normalize(TBN * tangentSpaceNormal);
    } else {
        N = normalize(worldNormal);
    }

    vec3 L = normalize(lightPos - worldPos);
    vec3 V = normalize(cameraPos - worldPos);

    float NdotL = max(dot(N, L), 0.0);


    vec3 R = reflect(-L, N);
    float RdotV = max(dot(R, V), 0.0);
    float specIntensity = pow(RdotV, shininess);

    vec3 ambient = k_a * baseColor;
    vec3 diffuse = k_d * NdotL * baseColor;
    vec3 specular = k_s * specIntensity * vec3(1.0);

    vec3 finalColor = ambient + diffuse + specular;
    color = vec4(finalColor, 1.0);
}
