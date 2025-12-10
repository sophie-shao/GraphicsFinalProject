#version 330 core

in vec2 uv;
out vec4 fragColor;

uniform sampler2D colorTex;
uniform sampler2D depthTex;
uniform sampler2D lutTex;
uniform float offset;
uniform float time;

uniform float lutSize;


uniform bool postProcessing;
uniform float near;
uniform float far;

uniform int ppMode;
uniform bool enableGrainOverlay;
uniform float grainSize;
uniform float grainOpacity;
uniform bool enablePixelate;
uniform bool enableBloom;

uniform float fieldPenalty;
uniform float forestPenalty;
uniform float mountainPenalty;
uniform float colorPenaltyStrength;




vec3 applyLUT(vec3 color)
{
    float size  = 64;
    float tiles = sqrt(size);
    float tileSize = 1.0 / tiles;

    vec3 c = clamp(color, 0.0, 1.0);
    float sizeF = size - 1.0;

    float slice = c.b * sizeF;
    float slice0 = floor(slice);
    float slice1 = min(slice0 + 1.0, sizeF);
    float t = slice - slice0;

    float x0 = mod(slice0, tiles);
    float y0 = floor(slice0 / tiles);

    float x1 = mod(slice1, tiles);
    float y1 = floor(slice1 / tiles);

    float u = (c.r * sizeF + 0.5) / size;
    float v = (c.g * sizeF + 0.5) / size;

    vec2 uv0 = vec2((x0 + u) * tileSize,
                    (y0 + v) * tileSize);

    vec2 uv1 = vec2((x1 + u) * tileSize,
                    (y1 + v) * tileSize);

    vec3 col0 = texture(lutTex, uv0).rgb;
    vec3 col1 = texture(lutTex, uv1).rgb;

    return mix(col0, col1, t);
}


vec3 visualizeDepth() {
    float d = texture(depthTex, uv).r;

    float z = d * 2.0 - 1.0;
    float linearDepth = (2.0 * near * far) / (far + near - z * (far - near));

    float depth01 = linearDepth / far;
    depth01 = pow(depth01, 0.5);

    return vec3(depth01);
}

vec3 sinWave(vec2 uv) {
    float waves = 4.0;
    float amp = 1.0 / 100.0;
    float xOffset = sin(uv.y * waves * 2.0 * 3.141593 + offset) * amp;

    float edgeFade = smoothstep(0.05, 0.10, uv.x) * smoothstep(0.95, 0.90, uv.x);
    xOffset *= edgeFade;

    vec2 warped = uv + vec2(xOffset, 0.0);
    warped = clamp(warped, 0.0, 1.0);

    return texture(colorTex, warped).rgb;
}
vec3 screenShake(vec2 uv)
{
    float strength = 0.01;

    float sx = cos(offset * 10.0) * strength;
    float sy = cos(offset * 15.0) * strength;

    float margin = 0.02;

    float fadeL = smoothstep(0.0, margin, uv.x);
    float fadeR = smoothstep(1.0, 1.0 - margin, uv.x);
    float fadeX = fadeL * fadeR;

    float fadeT = smoothstep(0.0, margin, uv.y);
    float fadeB = smoothstep(1.0, 1.0 - margin, uv.y);
    float fadeY = fadeT * fadeB;

    float fade = fadeX * fadeY;
    vec2 shaken = uv + fade * vec2(sx, sy);

    shaken = clamp(shaken, 0.0, 1.0);

    return texture(colorTex, shaken).rgb;
}


vec3 applyInkOutline(vec2 uv) {
    float px = 1.0 / 1024.0;
    float py = 1.0 / 768.0;

    vec3 c = texture(colorTex, uv).rgb;
    vec3 cx = texture(colorTex, uv + vec2(px, 0)).rgb;
    vec3 cy = texture(colorTex, uv + vec2(0, py)).rgb;

    float edge = length(c - cx) + length(c - cy);
    float line = smoothstep(0.1, 0.2, edge);

    return mix(c, vec3(0.0), line);
}

vec3 posterize(vec3 c) {
    float levels = 4.0;
    return floor(c * levels) / levels;
}

vec3 nightmareCartoon(vec3 c) {
    float levels = 4.0;
    vec3 post = floor(c * levels) / levels;
    post *= vec3(1.2, 1.15, 1.3);
    return clamp(post, 0.0, 1.0);
}


float hash(vec2 p) {
    return fract(sin(dot(p, vec2(23.3, 48.1))) * 23428.0);
}

vec3 staticNoise(vec2 uv, vec3 c) {
    float n = hash(uv * grainSize * time * 500.0);
    float grain = n * 2.0 - 1.0;

    float strength = 0.35 * grainOpacity;
    return clamp(c + grain * strength, 0.0, 1.0);
}


vec3 pixelate(vec2 uv) {
    float pixelCount = 330.0;
    
    vec2 texSize = textureSize(colorTex, 0);
    float aspectRatio = texSize.x / texSize.y;
    vec2 pixelScale = vec2(pixelCount, pixelCount / aspectRatio);
    
    vec2 pixelatedUV = floor(uv * pixelScale) / pixelScale;
    vec2 pixelCenter = pixelatedUV + (0.5 / pixelScale);
    vec3 color = texture(colorTex, pixelCenter).rgb;

    float vignetteFactor = 1.0;
    vec2 center = vec2(0.5, 0.5);
    vec2 offset = uv - center;
    offset.x /= aspectRatio;
    float distFromCenter = length(offset);
    
    float vignetteStart = 0.3;
    float vignetteEnd = 0.64;
    float vignetteStrength = 0.75;
    
    if (distFromCenter > vignetteStart) {
        float vignetteRange = vignetteEnd - vignetteStart;
        float vignetteProgress = (distFromCenter - vignetteStart) / vignetteRange;
        vignetteProgress = clamp(vignetteProgress, 0.0, 1.0);
        vignetteProgress = vignetteProgress * vignetteProgress;
        vignetteFactor = 1.0 - (vignetteProgress * vignetteStrength);
    }
    
    vec3 vignettedColor = color * vignetteFactor;
    
    vec3 colorPenalty = vec3(1.0);
    colorPenalty.r = 1.0 - (colorPenaltyStrength * (1.0 - fieldPenalty));
    colorPenalty.g = 1.0 - (colorPenaltyStrength * (1.0 - forestPenalty));
    colorPenalty.b = 1.0 - (colorPenaltyStrength * (1.0 - mountainPenalty));
    
    vec3 finalColor = vignettedColor * colorPenalty;
    
    vec3 bloomColor = vec3(0.0);
    if (enableBloom) {
        float maxChannel = max(max(finalColor.r, finalColor.g), finalColor.b);
        if (maxChannel > 0.8) {
            vec2 texSize = textureSize(colorTex, 0);
            vec2 texelSize = 1.0 / texSize;
            float blurRadius = 3.0;
            
            vec3 blurred = vec3(0.0);
            float weightSum = 0.0;
            
            for (int i = -3; i <= 3; i++) {
                for (int j = -3; j <= 3; j++) {
                    vec2 offset = vec2(float(i), float(j)) * texelSize * blurRadius;
                    vec2 sampleUV = clamp(uv + offset, 0.0, 1.0);
                    vec3 sampleColor = texture(colorTex, sampleUV).rgb;
                    
                    float sampleMax = max(max(sampleColor.r, sampleColor.g), sampleColor.b);
                    if (sampleMax > 0.8) {
                        float bloomStrength = smoothstep(0.8, 1.0, sampleMax);
                        float distance = length(vec2(float(i), float(j)));
                        float weight = 1.0 / (1.0 + distance * 0.5);
                        blurred += sampleColor * bloomStrength * weight;
                        weightSum += weight;
                    }
                }
            }
            
            if (weightSum > 0.0) {
                bloomColor = blurred / weightSum * 0.4;
            }
        }
    }
    
    return finalColor + bloomColor;
}

void main()
{
    vec3 c = texture(colorTex, uv).rgb;

    if (!postProcessing) {
        fragColor = vec4(c, 1.0);
        return;
    }

    vec3 processed = c;
    
    if (enablePixelate) {
        float pixelCount = 360.0;
        vec2 texSize = textureSize(colorTex, 0);
        float aspectRatio = texSize.x / texSize.y;
        vec2 pixelScale = vec2(pixelCount, pixelCount / aspectRatio);
        
        vec2 pixelatedUV = floor(uv * pixelScale) / pixelScale;
        vec2 pixelCenter = pixelatedUV + (0.5 / pixelScale);
        
        vec3 pixelatedInput = texture(colorTex, pixelCenter).rgb;
        vec3 pixelatedProcessed = pixelatedInput;
        
        if (ppMode == 1){
            pixelatedProcessed = sinWave(pixelatedUV);
        }
        else if (ppMode == 2){
            pixelatedProcessed = screenShake(pixelatedUV);
        }
        else if (ppMode == 3) {
            pixelatedProcessed = applyInkOutline(pixelatedUV);
        }
        else if (ppMode == 4) {
            pixelatedProcessed = nightmareCartoon(pixelatedInput);
        }
        else if (ppMode == 5) {
            pixelatedProcessed = staticNoise(pixelatedUV, pixelatedInput);
        }
        else if (ppMode == 6) {
            pixelatedProcessed = visualizeDepth();
        }
        else if (ppMode == 7){
            pixelatedProcessed = applyLUT(pixelatedInput);
        }
        
        vec2 center = vec2(0.5, 0.5);
        vec2 offset = uv - center;
        offset.x /= aspectRatio;
        float distFromCenter = length(offset);
        
        float vignetteStart = 0.02;
        float vignetteEnd = 0.53;
        float vignetteStrength = 0.7;
        
        float vignetteFactor = 1.0;
        if (distFromCenter > vignetteStart) {
            float vignetteRange = vignetteEnd - vignetteStart;
            float vignetteProgress = (distFromCenter - vignetteStart) / vignetteRange;
            vignetteProgress = clamp(vignetteProgress, 0.0, 1.0);
            vignetteProgress = vignetteProgress * vignetteProgress;
            vignetteFactor = 1.0 - (vignetteProgress * vignetteStrength);
        }
        
        vec3 vignettedColor = pixelatedProcessed * vignetteFactor;
        
        vec3 colorPenalty = vec3(1.0);
        colorPenalty.r = 1.0 - (colorPenaltyStrength * (1.0 - fieldPenalty));
        colorPenalty.g = 1.0 - (colorPenaltyStrength * (1.0 - forestPenalty));
        colorPenalty.b = 1.0 - (colorPenaltyStrength * (1.0 - mountainPenalty));
        
        vec3 finalColor = vignettedColor * colorPenalty;
        
        vec3 bloomColor = vec3(0.0);
        if (enableBloom) {
            float maxChannel = max(max(finalColor.r, finalColor.g), finalColor.b);
            if (maxChannel > 0.8) {
            vec2 texSize = textureSize(colorTex, 0);
            vec2 texelSize = 1.0 / texSize;
            float blurRadius = 3.0;
            
            vec3 blurred = vec3(0.0);
            float weightSum = 0.0;
            
            for (int i = -3; i <= 3; i++) {
                for (int j = -3; j <= 3; j++) {
                    vec2 offset = vec2(float(i), float(j)) * texelSize * blurRadius;
                    vec2 sampleUV = clamp(uv + offset, 0.0, 1.0);
                    
                    vec3 sampleColor = texture(colorTex, sampleUV).rgb;
                    
                    float sampleMax = max(max(sampleColor.r, sampleColor.g), sampleColor.b);
                    if (sampleMax > 0.8) {
                        float bloomStrength = smoothstep(0.8, 1.0, sampleMax);
                        float distance = length(vec2(float(i), float(j)));
                        float weight = 1.0 / (1.0 + distance * 0.5);
                        blurred += sampleColor * bloomStrength * weight;
                        weightSum += weight;
                    }
                }
            }
            
            if (weightSum > 0.0) {
                bloomColor = blurred / weightSum * 0.4;
            }
            }
        }
        
        processed = finalColor + bloomColor;
        } else {
        if (ppMode == 1){
            processed = sinWave(uv);
        }
        else if (ppMode == 2){
            processed = screenShake(uv);
        }
        else if (ppMode == 3) {
            processed = applyInkOutline(uv);
        }
        else if (ppMode == 4) {
            processed = nightmareCartoon(c);
        }
        else if (ppMode == 5) {
            processed = staticNoise(uv, c);
        }
        else if (ppMode == 6) {
            processed = visualizeDepth();
        }
        else if (ppMode == 7){
            processed = applyLUT(c);
        }

    }
    
    if (enableGrainOverlay) {
        processed = staticNoise(uv, processed);
    }

    fragColor = vec4(processed, 1.0);
}



