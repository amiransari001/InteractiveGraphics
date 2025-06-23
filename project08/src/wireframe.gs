#version 450 core

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

void main() {
    // Edge 0 → 1
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();
    EndPrimitive();

    // Edge 1 → 2
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();
    EndPrimitive();

    // Edge 2 → 0
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    EndPrimitive();
}

