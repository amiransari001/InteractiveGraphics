#version 450 core

layout(location = 0) out vec4 color; 
in vec3 frag_nrm; 
in vec4 m_pos; 

uniform vec3 Kd; 
uniform vec3 Ks; 
uniform float alpha; 

uniform vec3 lightDir; 
uniform float lightIntensity; 
uniform vec3 lightColor; 
uniform vec3 cameraPos; 


void main () 
{
    // color = vec4(0.0, 1.0, 0.0, 1.0);
    vec3 nrm = normalize(frag_nrm); 
    vec3 viewDir = normalize(cameraPos - vec3(m_pos.x, m_pos.y, m_pos.z)) ;
    vec3 h = normalize(lightDir + viewDir); 
    float phi = max(0.0, dot(nrm, h)); 

    float geom = max(0, dot(nrm, lightDir));
    vec3 clr = lightIntensity * (geom * Kd + Ks * pow(phi, alpha)) + 0.25 * Kd;
    
    // vec3 clr = lightIntensity * geom * Kd + 0.25 * Kd;
    // color = vec4(nrm, 1.0);
    color = vec4(clr, 1.0); 
}
