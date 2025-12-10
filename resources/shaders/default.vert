#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 uv;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

out vec3 worldPos;
out vec3 worldNormal;
out vec2 fragUV;
out mat3 TBN;

void main() {
    vec4 worldPosition4 = modelMatrix * vec4(position, 1.0);
    worldPos = worldPosition4.xyz;

    // Transform normal, tangent, and bitangent to world space
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    vec3 T = normalize(normalMatrix * tangent);
    vec3 B = normalize(normalMatrix * bitangent);
    vec3 N = normalize(normalMatrix * normal);

    // Gram-Schmidt re-orthogonalization
    T = normalize(T - dot(T, N) * N);
    B = normalize(cross(N, T));

    // Create TBN matrix for tangent space transformation
    TBN = mat3(T, B, N);

    worldNormal = N;
    fragUV = uv;

    gl_Position = projMatrix * viewMatrix * worldPosition4;
}
