#version 330 core

// Task 5: declare "in" variables for the world-space position and normal,
//         received post-interpolation from the vertex shader
in vec3 worldPos;
in vec3 worldNormal;
// Task 10: declare an out vec4 for your output color

out vec4 color;
// Task 12: declare relevant uniform(s) here, for ambient lighting
// Task 13: declare relevant uniform(s) here, for diffuse lighting
// Task 14: declare relevant uniform(s) here, for specular lighting
uniform float k_a;
uniform float k_d;
uniform float k_s;
uniform float shininess;
uniform vec3 lightPos;
uniform vec3 cameraPos;


void main() {
    // Remember that you need to renormalize vectors here if you want them to be normalized
    // Task 10: set your output color to white (i.e. vec4(1.0)). Make sure you get a white circle!
    //color = vec4(1.0f);

    // Task 11: set your output color to the absolute value of your world-space normals,
    //          to make sure your normals are correct.
    // color = vec4(abs(worldNormal), 1.0);

    // Task 12: add ambient component to output color
    // Task 13: add diffuse component to output color
    // Task 14: add specular component to output color
    vec3 N = normalize(worldNormal);
    vec3 L = normalize(lightPos - worldPos);
    vec3 V = normalize(cameraPos - worldPos);

    float NdotL = max(dot(N, L), 0.0);
    vec3 R = reflect(-L, N);
    float RdotV = max(dot(R, V), 0.0);
    float specIntensity = pow(RdotV, shininess);

    vec3 ambient = vec3(k_a);
    vec3 diffuse = k_d * NdotL * vec3(1.0);
    vec3 specular = k_s * specIntensity * vec3(1.0);

    vec3 finalColor = ambient+diffuse+specular;

    color = vec4(finalColor, 1.0);
}
