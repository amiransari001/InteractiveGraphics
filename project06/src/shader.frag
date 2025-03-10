#version 450 core

layout(location = 0) out vec4 color; 
// uniform sampler2D tex;
// in vec2 texCoord;
uniform samplerCube env;

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

    vec3 clr = lightIntensity * (geom * Kd + Ks * pow(phi, alpha)) + 0.25 *Kd;
    
    vec3 rdir = reflect(-viewDir, nrm); 
    // rdir = vec3(rdir.x, rdir.y, -rdir.z); 
    vec4 env_clr = texture(env, rdir)* vec4(Ks, 1.0);
    
    // vec4 tex_clr = texture(tex, texCoord);
    // vec4 tex_Ks = vec4(Ks, 1.0); 
    // vec4 clr = lightIntensity * (geom * tex_clr + tex_Ks * pow(phi, alpha)) + 0.25 * tex_clr;

    // vec3 clr = lightIntensity * geom * Kd + 0.25 * Kd;
    // color = vec4(nrm, 1.0);
    // color = vec4(clr, 1.0); 
    // color = clr; 
    color = normalize(vec4(clr, 1.0)) + normalize(env_clr); 
    //color = texture(env, rdir); 
    // color = texture(env, vec3(1.0, 0.0, 0.0));
}
