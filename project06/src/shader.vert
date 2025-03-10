#version 450 core

layout(location = 0) in vec3 pos; 
layout(location = 1) in vec3 nrm;
// layout(location = 2) in vec3 tCoord;

uniform bool swap; 
uniform mat4 mvp;
uniform mat4 m; 
uniform mat3 nrm_m;

out vec3 frag_nrm;
out vec4 m_pos; 
// out vec2 texCoord;

void main()
{
    vec3 modified_pos = pos;
    vec3 modified_nrm = nrm; 
    
    if (swap) {
        modified_pos = vec3(pos.x,pos.z, pos.y);
        modified_nrm = vec3(nrm.x, nrm.z, nrm.y);
    }

    frag_nrm = nrm_m * modified_nrm; 
    // frag_nrm = modified_nrm; 

    m_pos = m * vec4(modified_pos, 1.0);
    // texCoord = vec2(tCoord.x, tCoord.y);
    
    gl_Position = mvp * vec4(modified_pos, 1.0);
}