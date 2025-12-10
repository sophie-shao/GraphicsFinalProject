#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 projMatrix;
uniform mat4 mvMatrix;

out vec3 vNormal;

void main() {
    gl_Position = projMatrix * mvMatrix * vec4(position, 1.0);
    vNormal = normalize(normal);
}
