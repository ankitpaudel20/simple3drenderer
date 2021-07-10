#version 330 core

struct Material {
    sampler2D ambientMap;
    sampler2D diffuseMap;
    sampler2D specularMap;
    sampler2D normalMap;
    vec3 specularColor;
    float shininess;
    float ambientStrength;
    float diffuseStrength;
    float specularStrength;
    vec3 diffuseColor;
};

struct pointLight {
    vec3 position;
    float intensity;
    vec3 diffuseColor;
    vec3 ambientColor;

    float radius;

    float constant;
    float linear;
    float quadratic;

    samplerCube depthMap;
};

struct DirLight {
    vec3 direction;
    float intensity;
    vec3 color;
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
in vec3 tangentLightPos;
in vec3 tangentViewPos;
in vec3 tangentFragPos;

out vec4 final_color;

uniform int activePointLights;
uniform vec3 camPos;
uniform Material material;
uniform DirLight dirLight;
uniform pointLight pointLights[5];
uniform vec3 ambientLight;
uniform float ambientStrength;
uniform int doLightCalculations;
uniform float shadow_farplane;
uniform bool enable_shadows;
uniform bool enable_normals;
uniform mat4 viewProj;
uniform float bias1;


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffuseColor);
vec3 CalcPointLight(pointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor, vec3 tangentLightPos, vec3 tangentFragPos,float int_by_at);
vec3 CalcPointLight2(pointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor);

flat in float debug;

bool has_normal;
void main() {
    vec3 norm;
    has_normal = texture(material.normalMap, f_texCoord).rgb == vec3(0) ? false : true;
    norm=(enable_normals && has_normal) ? texture(material.normalMap, f_texCoord).rgb * 2.0 - 1.0 : f_normal;
    norm=normalize(norm);
 

    vec3 diffColor = material.diffuseColor;
    vec3 tex = vec3(texture(material.diffuseMap, f_texCoord));

    vec3 viewDir;

    if (enable_normals && has_normal)
        viewDir = normalize(tangentViewPos - tangentFragPos);
    else
        viewDir = normalize(camPos - f_position);

    vec3 result = vec3(0);

    if (tex != vec3(0)) {
        diffColor *= tex;
    }

    if (doLightCalculations == 1) {
       for (int i = 0; i < activePointLights; i++) {            
           float dist=distance(f_position,pointLights[i].position);     
           float int_by_at =pointLights[i].intensity/ (pointLights[i].constant + pointLights[i].linear * dist + pointLights[i].quadratic * (dist * dist));    
           if (int_by_at > 0.004){              
               result += CalcPointLight(pointLights[i], norm, f_position, viewDir, diffColor, tangentLightPos, tangentFragPos, int_by_at);
           }
       }
       result+=CalcDirLight(dirLight,norm,viewDir,diffColor);       
    } else
        result += diffColor;

    result += ambientLight * ambientStrength * diffColor;
    
    // if ((viewProj*vec4(f_position,1)).z<1){
    //     result=vec3(1);
    // }
    // else{
    //     result=vec3(0);
    // }

    final_color = vec4(result,1);    
}

float ShadowCalculation(vec3 fragPos, pointLight light, vec3 normal, vec3 lightDir) {
    // get vector between fragment position and light position

    vec3 fragToLight = fragPos - light.position;

    // ise the fragment to light vector to sample from the depth map

    float closestDepth = texture(light.depthMap, fragToLight).r;

    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= shadow_farplane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    // float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    // float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);  
    float bias = dot(normal, lightDir)*0.1;  

    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);

    return shadow;
}

vec3 CalcPointLight(pointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor, vec3 tangentLightPos, vec3 tangentFragPos, float int_by_at) {
    vec3 lightDir;

    if (has_normal && enable_normals)
        lightDir = normalize(tangentLightPos - tangentFragPos);
    else
        lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
   
    vec3 ambientColor = vec3(texture(material.ambientMap, f_texCoord));
    if (ambientColor == vec3(0)) {
        ambientColor = diffuseColor;
    }

    vec3 specularColor = vec3(texture(material.specularMap, f_texCoord));
    if (specularColor == vec3(0)) {
        specularColor = vec3(1);
    }

    vec3 ambient =  int_by_at * light.ambientColor * material.ambientStrength * diffuseColor;
    vec3 diffuse =  int_by_at * light.diffuseColor * material.diffuseStrength * diff * diffuseColor;
    vec3 specular = int_by_at * light.diffuseColor * material.specularStrength * spec * material.specularColor * specularColor;
   
    if (enable_shadows) {
        float shadow = ShadowCalculation(fragPos, light, normal, lightDir);
        return (ambient + (1.0 - shadow) * (diffuse + specular));
    } else
        return (ambient + diffuse + specular);   
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffuseColor) {

    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    
    vec3 diffuse = light.intensity * light.color * material.diffuseStrength * diff * diffuseColor;
    vec3 specular = light.intensity * light.color * material.specularStrength * spec * material.specularColor;
    return ( diffuse + specular);
}

vec3 calculateFlashLight(flashLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = -normalize(fragPos - light.position);

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // ambient
    vec3 ambient = light.ambient * light.ambientStrength * vec3(texture(material.diffuseMap, f_texCoord));

    // diffuse
    vec3 norm = normalize(f_normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * light.diffuseStrength * diff * vec3(texture(material.diffuseMap, f_texCoord));

    // specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * light.specularStrength * spec * vec3(texture(material.specularMap, f_texCoord));

    // spotlight (soft edges)
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    diffuse *= intensity;
    specular *= intensity;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}