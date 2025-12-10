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


// blooom
uniform sampler2D bloomBlur;
uniform float exposure;





vec3 applyLUT(vec3 color)
{
    float size  = 64;
    float tiles = sqrt(size);
    float tileSize = 1.0 / tiles;

    vec3 c = clamp(color, 0.0, 1.0);
    float sizeF = size - 1.0;

    //which two slices (blue axis)
    float slice = c.b * sizeF;
    float slice0 = floor(slice);
    float slice1 = min(slice0 + 1.0, sizeF);
    float t = slice - slice0;

    float x0 = mod(slice0, tiles);
    float y0 = floor(slice0 / tiles);

    float x1 = mod(slice1, tiles);
    float y1 = floor(slice1 / tiles);

    //UV inside each tile
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
    float linearDepth = (2.0*near*far) / (far+near-z * (far-near));
    float depth01 = linearDepth / far;

    // curve for visibility
    depth01 = pow(depth01, 0.5);

    return vec3(depth01);
}

// sinwave stuff
vec3 sinWave(vec2 uv) {
    // edit for num of waves
    float waves = 4.0;

    // amplitude
    float amp = 1.0 / 100.0;

    // phase shift
    float xOffset = sin(uv.y * waves * 2.0 * 3.141593 + offset) * amp;

    // added this cause it was distorting near the edges
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

    // fade same as above to keep from clipping
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


// harder outlines
vec3 applyInkOutline(vec2 uv) {
    float px = 1.0 / 1024.0;  // adjust to your viewport
    float py = 1.0 / 768.0;

    vec3 c = texture(colorTex, uv).rgb;
    vec3 cx = texture(colorTex, uv + vec2(px, 0)).rgb;
    vec3 cy = texture(colorTex, uv + vec2(0, py)).rgb;

    float edge = length(c - cx) + length(c - cy);
    float line = smoothstep(0.1, 0.2, edge);

    return mix(c, vec3(0.0), line);
}

vec3 posterize(vec3 c) {
    float levels = 4.0; // fewer = more stylized
    return floor(c * levels) / levels;
}

vec3 nightmareCartoon(vec3 c) {
    float levels = 4.0;
    vec3 post = floor(c * levels) / levels;

    // creepy
    post *= vec3(0.9, 0.85, 1.1);

    return clamp(post, 0.0, 1.0);
}


float hash(vec2 p) {
    // sin + large multiplier to get a random val
    // fract to keep only the 0 to 1
    return fract(sin(dot(p, vec2(23.3, 48.1))) * 23428.0);
}

vec3 staticNoise(vec2 uv, vec3 c) {
    float n = hash(uv * time * 500.0);
    float grain = n * 2.0 - 1.0; // hash gives 0-1, we make it -1 to 1 to make dark and light spots

    float strength = 0.35;// how strong the static is
    return clamp(c + grain * strength, 0.0, 1.0);
}

void main()
{

    //original rendered color
    vec3 c = texture(colorTex, uv).rgb;

    if (!postProcessing) {
        fragColor = vec4(c, 1.0);
        return;
    }

    vec3 processed = c;

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

    // fragColor = vec4(processed, 1.0);

    vec3 bloom = texture(bloomBlur, uv).rgb;
    vec3 hdrColor = processed + bloom;   // add bloom

    // // tone mapping (same as LearnOpenGL)
    // vec3 result = vec3(1.0) - exp(-hdrColor * 1.0f);

    // // gamma correction
    // float gamma = 2.2;
    // result = pow(result, vec3(1.0 / gamma));
    vec3 result = hdrColor;

    fragColor = vec4(result, 1.0);
    // fragColor = vec4(processed, 1.0);

}
