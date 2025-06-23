#version 450 core

layout(location = 0) out vec4 color; 

in vec3 frag_nrm; 
in vec4 m_pos; 

uniform vec3 Kd; 
uniform vec3 Ks; 
uniform float alpha; 

uniform vec3 lightPos;
uniform vec3 lightSpotDir; 
uniform float cos_cuttoff; 
uniform float lightIntensity; 
uniform vec3 lightColor; 

//ATTENTUATION TERMS
uniform float att_linear; 
uniform float att_quadratic; 


uniform vec3 cameraPos; 

uniform sampler2DShadow shadow;
in vec4 lightView_Position;

void main () 
{

    // CALC FRAG LIGHT DIRECTION
    vec3 lightDir = normalize(lightPos - vec3(m_pos.x, m_pos.y, m_pos.z));
    float lightDist = length(lightPos - vec3(m_pos.x, m_pos.y, m_pos.z)); 

    //  CALC THETA: ANGLE FROM SPOTDIR
    float theta = dot(lightDir, normalize(-lightSpotDir));

    float attenuation = 1.0 / (1.0 + att_linear * lightDist + att_quadratic * (lightDist * lightDist)); 

    vec3 clr = vec3(0.0, 0.0, 0.0); 
    if (theta > cos_cuttoff) {
        // SHADE
        vec3 nrm = normalize(frag_nrm); 
        vec3 viewDir = normalize(cameraPos - vec3(m_pos.x, m_pos.y, m_pos.z)) ;
        vec3 h = normalize(lightDir + viewDir); 
        float phi = max(0.0, dot(nrm, h)); 
        float geom = max(0, dot(nrm, lightDir));

        vec3 diffuse = lightIntensity * geom * Kd;
        vec3 specular = lightIntensity *  Ks * pow(phi, alpha);
        //vec3 ambient = 0.25 * Kd; 

        // clr = attenuation * (diffuse + specular + ambient); 
        clr = attenuation * (diffuse + specular); 
        // clr = diffuse + specular + ambient; 
        //clr = lightIntensity * (geom * Kd + Ks * pow(phi, alpha)) + 0.25 * Kd;
    } 
    //else {
        // JUST AMBIENT
        //clr = attenuation * 0.25 * Kd;
    //}

    vec3 ambient = attenuation * 0.25 * Kd; 
    float nrmDotLight = dot(frag_nrm, lightDir);
    //nrmDotLight = clamp(nrmDotLight, 0.0, 1.0);
    float bias = max(0.000639 * (1.0 - nrmDotLight), 0.00048);
    color = vec4(clr, 1.0); 
    vec4 biased_lightView_Position = vec4(lightView_Position.x, lightView_Position.y, lightView_Position.z - bias, lightView_Position.w);
    color *= textureProj( shadow, biased_lightView_Position );
    color += vec4(ambient, 1.0); 





    // color = vec4(0.0, 1.0, 0.0, 1.0);
    // vec3 nrm = normalize(frag_nrm); 
    // vec3 viewDir = normalize(cameraPos - vec3(m_pos.x, m_pos.y, m_pos.z)) ;
    // vec3 h = normalize(lightDir + viewDir); 
    // float phi = max(0.0, dot(nrm, h)); 
    // float geom = max(0, dot(nrm, lightDir));
    // vec3 clr = lightIntensity * (geom * Kd + Ks * pow(phi, alpha)) + 0.25 * Kd;
    
    
    // vec4 tex_clr = texture(tex, texCoord);
    // vec4 tex_Ks = vec4(Ks, 1.0); 
    //  vec4 clr = lightIntensity * (geom * tex_clr + tex_Ks * pow(phi, alpha)) + 0.25 * tex_clr;


    // vec3 clr = lightIntensity * geom * Kd + 0.25 * Kd;
    // color = vec4(nrm, 1.0);
    // color = vec4(clr, 1.0); 
    // color = clr; 
}
