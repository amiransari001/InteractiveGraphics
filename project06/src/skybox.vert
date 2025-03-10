#version 450 core
layout(location = 0) in vec3 skybox_pos;

out vec3 texCoord;

uniform mat4 p;
uniform mat4 v;

void main() {
    gl_Position = p * v * vec4(skybox_pos.x, skybox_pos.y, skybox_pos.z, 1.0); 
    texCoord = vec3(skybox_pos.x, skybox_pos.y, skybox_pos.z); 
}

// POS.XYWW FORCE BOX TO THE BACK WITHOUT DISABLING DEPTH