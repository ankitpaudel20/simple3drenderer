#version 330 core
layout(location = 0) in vec4 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texCoord;
layout(location = 3) in vec3 v_tangent;
layout(location = 4) in vec3 v_bitangent;

out vec3 f_position;
out vec3 f_normal;
out vec2 f_texCoord;
// out vec3 f_tangent;
// out vec3 f_bitangent;

//----------------------------
out vec3 tangentLightPos;
out vec3 tangentViewPos;
out vec3 tangentFragPos;
//----------------------------

uniform mat4 model;
uniform mat4 viewProj;

// Edited from here -------------

uniform vec3 camPos; //viewpos
struct pointLight {
    vec3 position; //lightpos
    float intensity;
    vec3 diffuseColor;
    vec3 ambientColor;

    float radius;
    float constant;
    float linear;
    float quadratic;
    samplerCube depthMap;
};
uniform int activePointLights;
uniform pointLight pointLights[5];

flat out float debug;

//---------------------------

void main() {
    vec4 pos = viewProj * model * v_position;
    gl_Position = pos;
    debug=pos.x;
    mat3 normalMatrix = transpose(inverse(mat3(model)));

    //----
    vec3 T = normalize(normalMatrix * v_tangent);
    vec3 N = normalize(normalMatrix * v_normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = transpose(mat3(T, B, N));
    // -------

    f_position = (model * v_position).xyz;
    f_normal = normalMatrix * v_normal;
    f_texCoord = v_texCoord;

    // ----------
    // first light only for now
    for (int i=0; i<activePointLights; i++){
            tangentLightPos = TBN * pointLights[i].position;
    }
    tangentViewPos = TBN * camPos;
    tangentFragPos = TBN * f_position;

    //----
}
