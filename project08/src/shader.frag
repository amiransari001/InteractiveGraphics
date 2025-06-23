#version 450 core

layout(location = 0) out vec4 color; 
uniform sampler2D normal_tex;

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

//uniform sampler2DShadow shadow;
in vec4 lightView_Position;
in vec3 frag_nrm; 
in vec3 frag_bitangent; 
in vec3 frag_tangent; 
in vec4 m_pos; 
in vec2 texCoord; 

void main () 
{
    vec3 N = normalize(frag_nrm);
    vec3 T = normalize(frag_tangent);
    T = normalize(T - N * dot(N, T)); // remove normal component from tangent
    vec3 B = normalize(cross(N, T));  // recompute bitangent as orthogonal to both

    mat3 TBN = mat3(T, B, N);
    //mat3 TBN = mat3(normalize(frag_tangent),normalize(frag_bitangent),normalize(frag_nrm));
    //mat3 TBN = mat3(frag_tangent,frag_bitangent,frag_nrm);

    vec3 normalTS = texture(normal_tex, texCoord).rgb * 2.0 - 1.0;
    vec3 normalWS = normalize(TBN * normalTS);
    

    // CALC FRAG LIGHT DIRECTION
    vec3 lightDir = normalize(lightPos - vec3(m_pos.x, m_pos.y, m_pos.z));
    float lightDist = length(lightPos - vec3(m_pos.x, m_pos.y, m_pos.z)); 

    if (dot(normalWS, lightDir) < 0.0) {
        normalWS = -normalWS;
    }

    //  CALC THETA: ANGLE FROM SPOTDIR
    float theta = dot(lightDir, normalize(-lightSpotDir));

    float attenuation = 1.0 / (1.0 + att_linear * lightDist + att_quadratic * (lightDist * lightDist)); 

    //vec3 frag_nrmx = texture(normal_tex, texCoord).rgb;
    //frag_nrmx = normalize(frag_nrmx * 2.0 - 1.0);

    vec3 clr = vec3(0.0, 0.0, 0.0); 
    if (theta > cos_cuttoff) {

        vec3 viewDir = normalize(cameraPos - vec3(m_pos.x, m_pos.y, m_pos.z)) ;
        vec3 h = normalize(lightDir + viewDir); 
        float phi = max(0.0, dot(normalWS, h)); 
        float geom = max(0, dot(normalWS, lightDir));

        vec3 diffuse = lightIntensity * geom * Kd;
        vec3 specular = lightIntensity *  Ks * pow(phi, alpha);
        //vec3 ambient = 0.25 * Kd; 
        
        clr = attenuation * (diffuse + specular); 
    } 
    vec3 ambient = attenuation * 0.25 * Kd; 
    clr += ambient; 
    color = vec4(clr, 1.0); 

    //float nrmDotLight = dot(normalWS, lightDir);
    //float bias = max(0.000639 * (1.0 - nrmDotLight), 0.00048);
    //vec4 biased_lightView_Position = vec4(lightView_Position.x, lightView_Position.y, lightView_Position.z - bias, lightView_Position.w);
    //color *= textureProj( shadow, biased_lightView_Position );
    //color += vec4(ambient, 1.0); 
}
