#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

//in vec4 tes_lightView_Position[]; 
in vec2 tes_texCoord[]; 
in vec3 tes_frag_nrm[]; 
in vec4 tes_m_pos[]; 
in vec3 tes_frag_tangent[];
in vec3 tes_frag_bitangent[];

//out vec4 lightView_Position; 
out vec3 frag_nrm; 
out vec3 frag_tangent;
out vec3 frag_bitangent;
out vec4 m_pos; 
out vec2 texCoord; 

void main() {
    for (int i = 0; i < 3; ++i) {
        //lightView_Position = tes_lightView_Position[i];
        frag_nrm = tes_frag_nrm[i]; 
        m_pos  = tes_m_pos[i]; 
        texCoord = tes_texCoord[i];
        frag_tangent = tes_frag_tangent[i];
        frag_bitangent = tes_frag_bitangent[i];
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
