#version 330 core

in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragUV;
in mat3 TBN;

out vec4 fragColor;

uniform vec3 cameraPos;
uniform sampler2D colorTexture;
uniform sampler2D normalMap;
uniform sampler2D bumpMap;
uniform bool useNormalMap;
uniform bool useBumpMap;
uniform float bumpStrength;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform float ka;
uniform float kd;
uniform float ks;
uniform float shininess;

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

void main() {
    vec3 baseColor = texture(colorTexture, fragUV).rgb;

    vec3 normal;
    vec3 tangentSpaceNormal = computeBumpNormal(bumpMap, fragUV, bumpStrength);
    normal = normalize(TBN * tangentSpaceNormal);

    vec3 lightDir = normalize(lightPos - fragPosition);
    vec3 viewDir = normalize(cameraPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, normal);

    vec3 ambient = ka * baseColor;
    vec3 diffuse = kd * max(dot(normal, lightDir), 0.0) * baseColor * lightColor;
    vec3 specular = ks * pow(max(dot(reflectDir, viewDir), 0.0), shininess) * lightColor;

    vec3 result = ambient + diffuse + specular;
    fragColor = vec4(result, 1.0);
}
