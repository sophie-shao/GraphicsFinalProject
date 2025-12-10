#version 330 core

in vec2 uv;
out vec4 FragColor;

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;
uniform float bloomIntensity;

void main()
{
    vec3 scene = texture(sceneTexture, uv).rgb;
    vec3 bloom = texture(bloomTexture, uv).rgb;
    
    vec3 result = scene + bloom * bloomIntensity;
    
    FragColor = vec4(result, 1.0);
}

