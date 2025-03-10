#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D renderedTexture;  // The render texture from the offscreen pass
uniform vec3 planeColorOffset;        // A small constant offset to differentiate the quad's color

void main()
{
    vec4 texColor = texture(renderedTexture, TexCoord);
    FragColor = texColor + vec4(0.1, 0.1, 0.1, 1.0);
}