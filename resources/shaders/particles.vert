#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

layout (location = 2) in vec3 instancePos;
layout (location = 3) in float instanceSize;
layout (location = 4) in vec4 instanceColor;
layout (location = 5) in int instanceType;

out vec2 TexCoords;
out vec4 Color;
flat out int Type;

uniform mat4 view;
uniform mat4 proj;

void main()
{
    vec3 camRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 camUp    = vec3(view[0][1], view[1][1], view[2][1]);

    vec3 worldPos =
        instancePos
        + camRight * aPos.x * instanceSize
        + camUp    * aPos.y * instanceSize;

    gl_Position = proj * view * vec4(worldPos, 1.0);

    TexCoords = aUV;
    Color = instanceColor;
    Type = instanceType;
}





