#version 330 core

out vec2 TexCoord;
uniform mat4 mvp;

void main()
{
    // Define positions for a full-screen quad
    //vec2 positions[4] = vec2[](vec2(-4.0, -3.0),vec2( 4.0, -3.0),vec2(-4.0,  3.0),vec2( 4.0,  3.0));

    vec2 positions[4] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 1.0, -1.0),
        vec2(-1.0,  1.0),
        vec2( 1.0,  1.0)
    );
    
    // Corresponding texture coordinates
    vec2 texCoords[4] = vec2[](
        vec2(0.0, 0.0),
        vec2(1.0, 0.0),
        vec2(0.0, 1.0),
        vec2(1.0, 1.0)
    );
    
    gl_Position = mvp * vec4(positions[gl_VertexID], 0.0, 1.0);
    TexCoord = texCoords[gl_VertexID];
}
