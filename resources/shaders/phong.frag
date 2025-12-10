#version 330 core

struct Light {
    int type;
    vec3 position;
    vec3 direction;
    vec3 color;
    vec3 function;
    float angle;
    float penumbra;
};

struct Material {
    vec4 cAmbient;
    vec4 cDiffuse;
    vec4 cSpecular;
    float shininess;
};



in vec3 worldPos;
in vec3 worldNormal;

out vec4 color;

uniform int numLights;
uniform Light lights[8];

uniform vec3 cameraPos;
uniform Material material;

uniform float k_a;
uniform float k_d;
uniform float k_s;

vec3 computeLightContribution(vec3 N, vec3 V, vec3 worldPos, Light light) {
    vec3 L;
    float attenuation = 1.0;

    if (light.type == 1) {
        L = normalize(-light.direction);
    }

    else {
        vec3 toLight = light.position - worldPos;
        float dist = length(toLight);
        L = normalize(toLight);

        float c = light.function.x;
        float lin = light.function.y;
        float q = light.function.z;

        float denom = c + lin * dist + q * dist * dist;
        if (abs(denom) > 1.0)
            attenuation = 1.0 / denom;

        if (light.type == 2) {
            float theta = acos(dot(normalize(-light.direction), normalize(L)));

            float inner = light.angle - light.penumbra;
            float outer = light.angle;
            float intensity = 0.0;

            if (theta < inner) {
                intensity = 1.0;
            } else if (theta > outer) {
                intensity = 0.0;
            } else {
                float ratio = (theta - inner) / (outer - inner);
                float falloff = -2.0 * pow(ratio, 3.0) + 3.0 * pow(ratio, 2.0);
                intensity = 1.0 - falloff;
            }

            attenuation *= clamp(intensity, 0.0, 1.0);
        }
    }


    float NdotL = max(dot(N, L), 0.0);
    if (NdotL <= 0.0) return vec3(0.0);

    vec3 R = reflect(-L, N);
    float RdotV = max(dot(R, V), 0.0);
    float spec = max(pow(RdotV, material.shininess),0.0);

    vec3 diffuse = material.cDiffuse.rgb * k_d * NdotL * light.color;
    vec3 specular = material.cSpecular.rgb * k_s * spec * light.color;

    return attenuation * (diffuse + specular);
}

void main() {
    vec3 N = normalize(worldNormal);
    vec3 V = normalize(cameraPos - worldPos);

    vec3 total = material.cAmbient.rgb * k_a;

    for (int i = 0; i < numLights; ++i) {
        total += computeLightContribution(N, V, worldPos, lights[i]);
    }

    color = vec4(total, 1.0);
}
