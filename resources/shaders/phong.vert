#version 330 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;

out vec3 worldPos;
out vec3 worldNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

void main() {
    vec4 worldPosition4 = modelMatrix * vec4(pos, 1.0);
    worldPos = worldPosition4.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    worldNormal = normalize(normalMatrix * normal);

    gl_Position = projMatrix * viewMatrix * worldPosition4;
}
