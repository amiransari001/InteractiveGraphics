#version 450 core

layout(location = 0) in vec3 pos; 

uniform bool swap; 
uniform mat4 mvp; 

void main()
{
    vec3 modified_pos = pos;
    
    if (swap) {
        modified_pos = vec3(pos.x,pos.z, pos.y);
    }

    gl_Position = mvp * vec4(modified_pos, 1.0); 
}