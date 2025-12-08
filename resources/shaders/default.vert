#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragNormal;
out vec3 fragPosition;
out vec2 fragUV;
out mat3 TBN;

void main() {

    fragPosition = vec3(model * vec4(position, 1.0));

    fragUV = uv;

    mat3 normalMatrix = mat3(transpose(inverse(model)));

    vec3 T = normalize(normalMatrix * tangent);
    vec3 B = normalize(normalMatrix * bitangent);
    vec3 N = normalize(normalMatrix * normal);
    TBN = mat3(T, B, N);

    fragNormal = N;

    gl_Position = projection * view * model * vec4(position, 1.0);
}
