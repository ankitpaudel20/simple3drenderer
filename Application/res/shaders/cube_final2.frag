#version 330 core

struct Material {
    sampler2D diffuseMap;
    sampler2D specularMap;
    vec3 specularColor;
    float shininess;
    float ambientStrength;
    float diffuseStrength;
    float specularStrength;
};

struct pointLight {
    vec3 position;
    float intensity;
    vec3 diffusecolor;
    vec3 specularcolor;

    float constant;
    float linear;
    float quadratic;
};

struct DirLight {
    vec3 direction;
    float intensity;

    vec3 diffusecolor;
    vec3 specularcolor;
};

struct flashLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    vec3 ambient;
    float ambientStrength;
    vec3 diffuse;
    float diffuseStrength;
    vec3 specular;
    float specularStrength;
    
    float constant;
    float linear;
    float quadratic;
};
  


in vec3 f_position;
in vec3 f_normal;
in vec2 f_texCoord;
in vec3  f_tangent;
in vec3 f_bitangent;


out vec4 final_color;



#define NR_POINT_LIGHTS 1
uniform vec3 camPos;
uniform Material material;
uniform DirLight dirLight;
uniform pointLight pointLights[NR_POINT_LIGHTS];
uniform vec3 ambientLight;


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(pointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{

    vec3 norm = normalize(f_normal);
    vec3 viewDir = normalize(camPos - f_position);

    vec3 result=vec3(0);

    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, f_position, viewDir);

    // result+=CalcDirLight(dirLight,norm,viewDir);

    final_color = vec4(result, 1.0);
}

vec3 CalcPointLight(pointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = -normalize(fragPos-light.position );

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient =attenuation * light.intensity * ambientLight * material.ambientStrength * vec3(texture(material.diffuseMap, f_texCoord));
    vec3 diffuse =attenuation * light.intensity * light.diffusecolor * material.diffuseStrength * diff * vec3(texture(material.diffuseMap, f_texCoord));
    vec3 specular = attenuation * light.intensity * light.specularcolor * material.specularStrength * spec * material.specularColor * vec3(texture(material.specularMap, f_texCoord));
    return (ambient + diffuse + specular);
}


vec3 CalcPointLight2(pointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = -normalize(fragPos-light.position );

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    
    vec3 ambient =attenuation * light.intensity * ambientLight * material.ambientStrength *  vec3(1.0,0.0,0.0);   
    vec3 diffuse =attenuation * light.intensity * light.diffusecolor * material.diffuseStrength * diff * vec3(1.0,0.0,0.0); 
    vec3 specular = attenuation * light.intensity * light.specularcolor * material.specularStrength * spec * material.specularColor * vec3(1.0,1.0,1.0);
    return (ambient + diffuse + specular);
}


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.intensity * ambientLight * material.ambientStrength * vec3(texture(material.diffuseMap, f_texCoord));
    vec3 diffuse =light.intensity * light.diffusecolor * material.diffuseStrength * diff * vec3(texture(material.diffuseMap, f_texCoord));
    vec3 specular = light.intensity * light.specularcolor * material.specularStrength * spec * material.specularColor * vec3(texture(material.specularMap, f_texCoord));
    return (ambient + diffuse + specular);
}

vec3 calculateFlashLight(flashLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
    vec3 lightDir = -normalize(fragPos-light.position );

    
       // attenuation
        float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));  

        // ambient
    vec3 ambient = light.ambient*light.ambientStrength * vec3(texture(material.diffuseMap, f_texCoord));
        
        // diffuse 
    vec3 norm = normalize(f_normal);
        float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse =light.diffuse*light.diffuseStrength * diff * vec3(texture(material.diffuseMap, f_texCoord));

        // specular
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular *light.specularStrength* spec * vec3(texture(material.specularMap, f_texCoord));
        
    // spotlight (soft edges)
    float theta     = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
      diffuse  *= intensity;
    specular *= intensity;

       

        ambient  *= attenuation; 
        diffuse   *= attenuation;
        specular *= attenuation;   

    return (ambient + diffuse + specular);

}