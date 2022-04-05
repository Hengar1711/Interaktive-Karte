#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec2 aOffset;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
    gl_Position = projection * view * model * vec4(aPos + aOffset, 0.0, 1.0);
}