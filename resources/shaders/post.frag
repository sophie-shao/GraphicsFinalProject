#version 330 core

in vec2 uv;
out vec4 outColor;

uniform sampler2D colorTexture;
uniform sampler2D depthTexture;

uniform float nearPlane;
uniform float farPlane;
uniform vec3 camPos;

uniform int enableFog;
uniform int enableFlashlight;
uniform vec3 fogColor;
uniform float fogIntensity;

uniform vec3 flashlightPos;
uniform vec3 flashlightDir;
uniform float flashlightConeAngle;
uniform vec3 flashlightColor;

uniform mat4 inverseViewMatrix;
uniform mat4 inverseProjectionMatrix;

float linearizeDepth(float depth){
    float z = depth * 2.0 - 1.0;
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

vec3 reconstructWorldPos(float depth, vec2 uv){
    vec4 clipPos;
    clipPos.xy = uv * 2.0 - 1.0;
    clipPos.z  = depth * 2.0 - 1.0;
    clipPos.w  = 1.0;

    vec4 viewPos = inverseProjectionMatrix * clipPos;
    viewPos /= viewPos.w;

    vec4 worldPos = inverseViewMatrix * viewPos;
    return worldPos.xyz;
}

vec3 addFog(vec3 sceneColor, float depth){
    if (enableFog == 0) {
        return sceneColor;
    }
    
    vec3 worldPos = reconstructWorldPos(depth, uv);
    
    vec3 horizontalOffset = worldPos - camPos;
    horizontalOffset.y = 0.0;
    float horizontalDist = length(horizontalOffset);
    
    vec3 viewRay = worldPos - camPos;
    vec3 viewDir = normalize(viewRay);
    float viewUp = viewDir.y;
    
    const float CLEAR_RADIUS = 1.5;
    float fogEnd = farPlane * 0.8;
    float fogStart = 5.0;
    
    float distFactor = 1.0;
    if (horizontalDist > fogStart) {
        distFactor = clamp((fogEnd - horizontalDist) / (fogEnd - fogStart), 0.0, 1.0);
    }
    
    float clearFactor = 1.0;
    if (horizontalDist < CLEAR_RADIUS) {
        clearFactor = 1.0 - smoothstep(0.0, CLEAR_RADIUS, horizontalDist);
    } else {
        clearFactor = 0.0;
    }
    
    float finalFactor = max(distFactor, clearFactor);
    
    float verticalFactor = 1.0;
    if (viewUp > 0.15) {
        verticalFactor = 1.0 - smoothstep(0.15, 0.85, viewUp) * 0.4;
    }
    finalFactor = mix(finalFactor, finalFactor * verticalFactor, 0.75);
    
    finalFactor = pow(finalFactor, 1.0 / max(fogIntensity, 0.1));
    finalFactor = pow(finalFactor, 0.5);
    
    return fogColor * (1.0 - finalFactor) + sceneColor * finalFactor;
}


vec3 addFlashlight(vec3 prevColor){
    if (enableFlashlight == 0) {
        return prevColor;
    }
    
    float rawDepth = texture(depthTexture, uv).r;
    vec3 fragPos = reconstructWorldPos(rawDepth, uv);
    
    vec3 viewRay = fragPos - camPos;
    float viewDist = length(viewRay);
    vec3 viewDir = normalize(viewRay);
    
    vec3 flashDir = normalize(flashlightDir);
    float cosConeAngle = cos(flashlightConeAngle);
    float cosConeInner = cos(flashlightConeAngle * 0.95);
    
    vec3 beamColor = vec3(0.0);
    
    const int NUM_STEPS = 20;
    float maxDistance = 20.0;
    
    const float WEIGHT = 0.05;
    const float EXPOSURE = 0.7;
    
    for (int i = 0; i < NUM_STEPS; i++) {
        float t = float(i) / float(NUM_STEPS);
        float tNext = float(i + 1) / float(NUM_STEPS);
        
        vec3 samplePos = camPos + viewDir * (t * min(viewDist, maxDistance));
        vec3 toFrag = samplePos - flashlightPos;
        float dist = length(toFrag);
        vec3 L = normalize(toFrag);
        
        float cosAngle = dot(L, flashDir);
        
        if (cosAngle > cosConeAngle) {
            float coneMask = smoothstep(cosConeAngle, cosConeInner, cosAngle);
            
            float attenuation = 1.0 / (1.0 + dist * dist * 0.1);
            
            float stepSize = (tNext - t) * min(viewDist, maxDistance);
            float stepWeight = smoothstep(0.0, stepSize * 2.0, stepSize);
            
            float edgeFeather = smoothstep(cosConeAngle * 0.85, cosConeInner, cosAngle);
            coneMask *= edgeFeather;
            
            beamColor += flashlightColor * coneMask * attenuation * WEIGHT * stepWeight;
        }
    }
    
    return prevColor + beamColor * EXPOSURE;
}

void main() {
    vec3 sceneColor = texture(colorTexture, uv).rgb;
    float depth = texture(depthTexture, uv).r;
    
    vec3 color = addFog(sceneColor, depth);
    color = addFlashlight(color);
    
    outColor = vec4(color, 1.0);
}

