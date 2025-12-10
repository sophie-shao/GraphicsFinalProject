#version 330 core



const int LIGHT_MAX = 8;

struct Light {
    int type;
    vec4 pos;
    vec4 dir;
    vec4 color;
    float angle;
    vec3 atten;
    float penumbra;
};

uniform Light lights[LIGHT_MAX];
uniform int lightCount;
in vec3 worldPos;
in vec3 worldNorm;

uniform Light oneLight;

// out vec4 fragColor;
// // MRT outputs for bloom (matches LearnOpenGL)
layout(location = 0) out vec4 fragColor;     // normal scene
layout(location = 1) out vec4 brightColor;   // bright-pass for bloom


uniform float ka;
uniform vec4 cAmb;

uniform float kd;
// uniform vec3 lightPos;
uniform vec4 cDiff;

uniform float ks;
uniform float shininess;
uniform vec3 camPos;
uniform vec4 cSpec;

void main() {
    // Remember that you need to renormalize vectors here if you want them to be normalized

    // Task 10: set your output color to white (i.e. vec4(1.0)). Make sure you get a white circle!
    // fragColor = vec4(1.0f);

    // Task 11: set your output color to the absolute value of your world-space normals,
             // to make sure your normals are correct.

    // fragColor = vec4(abs(worldNorm), 1.0f);

    // Task 12: add ambient component to output color
    // fragColor = vec4(ka, ka, ka, 1.0f);

    // // Task 13: add diffuse component to output color
    // vec3 lightDir = normalize(oneLight.pos - worldPos);
    // float diffuse = kd * clamp(dot(normalize(worldNorm), lightDir), 0, 1);
    // fragColor += vec4(diffuse, diffuse, diffuse, 0.0f);

    // // Task 14: add specular component to output color
    // vec3 reflected = normalize(reflect(-lightDir, worldNorm));
    // vec3 camDir = normalize(camPos - worldPos);
    // float spec = ks * pow(clamp(dot(reflected, camDir), 0, 1), shininess);
    // fragColor += vec4(spec, spec, spec, 0.0f);


    vec3 illumination = vec3(0.0f, 0.0f, 0.0f);
    illumination += vec3(cAmb) * ka;

    for (int i = 0; i < lightCount; i++){
        Light light = lights[i];
        // light = oneLight;

        vec3 n = normalize(worldNorm);
        vec3 v = normalize(camPos.xyz - worldPos);


        if (light.type == 1) { // directional

            vec3 l = normalize(-light.dir.xyz);

            float NdotL = max(dot(n, l), 0.0f);
            vec3 diffuse = vec3(cDiff) * kd * NdotL * vec3(light.color);
            illumination += diffuse;

            vec3 r = normalize(2.0f * NdotL * n - l);
            float RdotV = clamp(dot(r, v), 0.0f, 1.0f);
            float specIntensity = 1.0f;
            if (shininess != 0) {
                specIntensity = float(pow(RdotV, shininess));
            }
            vec3 spec = ks * vec3(cSpec) * specIntensity * vec3(light.color);
            illumination += spec;
        }
        else if (light.type == 0) { // point light
            vec4 lPos = light.pos;
            float distance = length(vec3(lPos) - worldPos);

            vec3 attens = light.atten;
            float denom = attens[0] + attens[1]*distance + attens[2]*distance*distance;
            float atten = clamp(1.0f / max(denom, 1e-6f), 0.0f, 1.0f);
            vec3 l = normalize(vec3(lPos) - worldPos);

            float NdotL = max(dot(n, l), 0.0f);
            vec3 diffuse = atten * vec3(cDiff) * kd * NdotL * vec3(light.color);
            illumination += diffuse;


            vec3 r = normalize(2.0f * NdotL * n - l);
            float RdotV = clamp(dot(r, v), 0.0f, 1.0f);
            float specIntensity = 1.0f;
            if (shininess != 0) {
                specIntensity = float(pow(RdotV, shininess));
            }
            vec3 spec = atten * ks * vec3(cSpec) * specIntensity * vec3(light.color);
            illumination += spec;
        }
        else if (light.type == 2){ // spot light
            vec4 lPos = light.pos;
            float distance = length(vec3(lPos) - worldPos);

            vec3 attens = light.atten;
            float denom = attens[0] + attens[1]*distance + attens[2]*distance*distance;
            float atten = clamp((1.0f / max(denom, 1e-6f)), 0.0f, 1.0f);
            vec3 l = normalize(vec3(lPos) - worldPos);

            float falloff = 1.0f;
            // angular falloff for spot light
            float thetaOut = light.angle; // the angle between outer cone boundary and spotlight direction
            float thetaIn = light.angle - light.penumbra; //  the angle between inner cone boundary and spotlight direction
            // the angle between current direction and spotlight direction
            float x = acos(dot(normalize(-l), normalize(vec3(light.dir)))); // if both vec are normalized, dot product is cosine
            if (x <= thetaIn) {
                falloff = 1.0f;
            }
            else if (x >= thetaOut){
                falloff = 0.0f;
            }
            else {
                float thetaCalc = (x - thetaIn) / (thetaOut - thetaIn);
                falloff = 1 - (-2.0f * pow(thetaCalc, 3) + 3 * pow(thetaCalc, 2));
            }

            atten *= falloff;


            float NdotL = max(dot(n, l), 0.0f);
            vec3 diffuse = atten * vec3(cDiff) * kd * NdotL * vec3(light.color);
            illumination += diffuse;

            vec3 r = normalize(2.0f * NdotL * n - l);
            float RdotV = clamp(dot(r, v), 0.0f, 1.0f);
            float specIntensity = 1.0f;
            if (shininess != 0) {
                specIntensity = float(pow(RdotV, shininess));
            }
            vec3 spec = atten * ks * vec3(cSpec) * specIntensity * vec3(light.color);
            illumination += spec;
        }
        else {
            // nothing cause there's some error ig lmao
        }
    }
    fragColor = vec4(illumination, 1.0f);

    // for bloom
    float brightness = dot(fragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
        if(brightness > 1.0)
            brightColor = vec4(fragColor.rgb, 1.0);
        else
            brightColor = vec4(0.0, 0.0, 0.0, 1.0);
    // fragColor = brightColor;

    // fragColor = vec4(0.0, 1.0, 0.0, 1.0);

    // fragColor = vec4(abs(worldNorm), 1.0);

}

