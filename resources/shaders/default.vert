#version 330 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;

out vec3 worldPos;
out vec3 worldNormal;
out vec2 fragUV;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 worldPosition4 = model * vec4(pos, 1.0);
    worldPos = worldPosition4.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    worldNormal = normalize(normalMatrix * normal);

    // Generate UVs based on position (planar mapping)
    vec3 absNormal = abs(worldNormal);
    if (absNormal.y > 0.9) { // Top/Bottom face
        fragUV = vec2(pos.x, pos.z);
    } else if (absNormal.x > 0.9) { // Left/Right face
        fragUV = vec2(pos.z, pos.y);
    } else { // Front/Back face
        fragUV = vec2(pos.x, pos.y);
    }

    // Generate tangent space for axis-aligned cubes
    vec3 T, B;
    if (absNormal.y > 0.9) {
        T = vec3(1, 0, 0);
        B = vec3(0, 0, 1);
    } else if (absNormal.x > 0.9) {
        T = vec3(0, 0, 1);
        B = vec3(0, 1, 0);
    } else {
        T = vec3(1, 0, 0);
        B = vec3(0, 1, 0);
    }
    T = normalize(normalMatrix * T);
    B = normalize(normalMatrix * B);
    TBN = mat3(T, B, worldNormal);

    gl_Position = projection * view * worldPosition4;
}
