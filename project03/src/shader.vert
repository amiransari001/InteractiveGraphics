#version 450 core

layout(location = 0) in vec3 pos; 
layout(location = 1) in vec3 nrm;

uniform bool swap; 
uniform mat4 mvp;
uniform mat4 mv; 
uniform mat3 nrm_mv;

out vec3 frag_nrm;
out vec4 mv_pos; 

void main()
{
    vec3 modified_pos = pos;
    vec3 modified_nrm = nrm; 
    
    if (swap) {
        modified_pos = vec3(pos.x,pos.z, pos.y);
        modified_nrm = vec3(nrm.x, nrm.z, nrm.y);
    }

    frag_nrm = nrm_mv * modified_nrm; 
    mv_pos = mv * vec4(modified_pos, 1.0);
    
    gl_Position = mvp * vec4(modified_pos, 1.0);
}