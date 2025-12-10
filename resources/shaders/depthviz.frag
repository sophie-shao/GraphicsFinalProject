#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D depthTexture;
uniform float nearPlane;
uniform float farPlane;

void main() {
    float depth = texture(depthTexture, TexCoord).r;
    
    if (depth < 0.0001) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    
    float linearDepth = (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - (depth * 2.0 - 1.0) * (farPlane - nearPlane));
    float normalizedDepth = clamp((linearDepth - nearPlane) / (farPlane - nearPlane), 0.0, 1.0);
    
    FragColor = vec4(vec3(1.0 - normalizedDepth), 1.0);
}
