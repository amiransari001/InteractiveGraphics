#version 450 core

layout(location = 0) in vec3 pos; 
layout(location = 1) in vec2 tCoord;

out vec2 vs_texCoord;

void main()
{
    gl_Position = vec4(pos, 1.0);
    vs_texCoord = tCoord; 
}