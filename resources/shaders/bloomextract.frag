#version 330 core

in vec2 uv;
out vec4 FragColor;

uniform sampler2D sceneTexture;
uniform float bloomThreshold;

void main()
{
    vec3 color = texture(sceneTexture, uv).rgb;
    
    float brightness = max(max(color.r, color.g), color.b);
    
    if (brightness > bloomThreshold) {
        float bloomStrength = smoothstep(bloomThreshold, 1.0, brightness);
        bloomStrength = pow(bloomStrength, 0.5);
        FragColor = vec4(color * bloomStrength, 1.0);
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}

