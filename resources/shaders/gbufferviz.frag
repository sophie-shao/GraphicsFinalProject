#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D gbufferTexture;
uniform int visualizationMode;

void main() {
    vec4 sample = texture(gbufferTexture, TexCoord);
    
    if (visualizationMode == 1) {
        // Position: Show world positions as color
        vec3 pos = sample.rgb;
        // Normalize position to visible range by using modulo and scaling
        vec3 color = mod(pos, 20.0) / 20.0; // Create repeating pattern every 20 units
        FragColor = vec4(color, 1.0);
    } else if (visualizationMode == 2) {
        // Normal: Map from [-1,1] to [0,1] for visualization
        vec3 normal = sample.rgb;
        // Check if normal is valid (not all zeros)
        if (length(normal) > 0.001) {
            normal = normalize(normal);
            FragColor = vec4(normal * 0.5 + 0.5, 1.0);
        } else {
            FragColor = vec4(0.0, 0.0, 0.0, 1.0); // Black if invalid
        }
    } else if (visualizationMode == 3) {
        // Albedo: Display color as-is
        FragColor = vec4(sample.rgb, 1.0);
    } else if (visualizationMode == 4) {
        // Velocity: Show velocity magnitude
        vec2 velocity = sample.rg;
        float magnitude = length(velocity);
        // Scale to make it visible - velocity is typically small
        float scaled = magnitude * 10.0; // Scale up to make visible
        FragColor = vec4(vec3(clamp(scaled, 0.0, 1.0)), 1.0);
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}

