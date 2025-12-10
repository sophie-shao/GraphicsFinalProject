#version 330 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 tangent;
layout(location=3) in vec3 bitangent;
layout(location=4) in vec2 uv;

out vec3 worldPos;
out vec3 worldNormal;
out vec2 fragUV;
out vec4 currentScreenPos;
out vec4 previousScreenPos;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 prevViewProjMatrix;

void main() {
    vec4 worldPosition4 = modelMatrix * vec4(pos, 1.0);
    worldPos = worldPosition4.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    worldNormal = normalize(normalMatrix * normal);

    currentScreenPos = projMatrix * viewMatrix * worldPosition4;
    previousScreenPos = prevViewProjMatrix * worldPosition4;
    
    fragUV = uv;
    
    gl_Position = currentScreenPos;
}

