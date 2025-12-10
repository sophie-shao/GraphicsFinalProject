#version 330 core

in vec2 fragTexCoord;
out vec4 color;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gVelocity;
uniform sampler2D sceneTexture;

uniform int numSamples;

void main() {
    vec2 velocity = texture(gVelocity, fragTexCoord).rg;
    float speed = length(velocity);
    
    if (speed < 0.001) {
        color = texture(sceneTexture, fragTexCoord);
        return;
    }
    
    float maxVelocity = 0.1;
    if (speed > maxVelocity) {
        velocity = normalize(velocity) * maxVelocity;
        speed = maxVelocity;
    }
    
    vec3 result = texture(sceneTexture, fragTexCoord).rgb;
    float blurScale = min(speed * 15.0, 1.0);
    int samples = max(1, int(float(numSamples) * blurScale));
    
    for (int i = 1; i <= samples; ++i) {
        vec2 offset = velocity * (float(i) / float(samples + 1));
        vec2 sampleCoord = fragTexCoord - offset;
        sampleCoord = clamp(sampleCoord, vec2(0.0), vec2(1.0));
        result += texture(sceneTexture, sampleCoord).rgb;
    }
    
    result /= float(samples + 1);
    color = vec4(result, 1.0);
}

