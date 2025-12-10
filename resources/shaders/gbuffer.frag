#version 330 core

in vec3 worldPos;
in vec3 worldNormal;
in vec2 fragUV;
in vec4 currentScreenPos;
in vec4 previousScreenPos;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gAlbedo;
layout(location = 3) out vec2 gVelocity;

struct Material {
    vec4 cAmbient;
    vec4 cDiffuse;
    vec4 cSpecular;
    float shininess;
};

uniform Material material;
uniform sampler2D colorTexture;
uniform bool useColorTexture;

void main() {
    gPosition = vec4(worldPos, 1.0);
    gNormal = normalize(worldNormal);
    
    if (useColorTexture) {
        gAlbedo = texture(colorTexture, fragUV).rgb;
    } else {
        gAlbedo = material.cDiffuse.rgb;
    }
    
    vec2 currentNDC = vec2(0.0);
    vec2 previousNDC = vec2(0.0);
    
    if (abs(currentScreenPos.w) > 0.0001) {
        currentNDC = (currentScreenPos.xy / currentScreenPos.w) * 0.5 + 0.5;
    }
    
    if (abs(previousScreenPos.w) > 0.0001) {
        previousNDC = (previousScreenPos.xy / previousScreenPos.w) * 0.5 + 0.5;
    }
    
    vec2 velocity = currentNDC - previousNDC;
    gVelocity = clamp(velocity, vec2(-1.0), vec2(1.0));
}

