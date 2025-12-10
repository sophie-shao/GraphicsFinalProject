#version 330 core

in vec2 TexCoords;
in vec4 Color;
flat in int Type;

out vec4 FragColor;

uniform sampler2D sprite;

void main()
{
    vec4 texColor = texture(sprite, TexCoords);
    if (texColor.a < 0.05)
        discard;

    FragColor = texColor * Color;
}





