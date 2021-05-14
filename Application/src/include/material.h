//
// Created by ankit on 5/9/2021.
//
#include "core.h"
#include "Texture.h"


#ifndef OPENGL_CORE_MATERIAL_H
#define OPENGL_CORE_MATERIAL_H

struct Material {
    float AmbientStrength = 0.1;
    float DiffuseStrength = 1;
    float SpecularStrength = 1;
    vec3 specularColor = vec3(1);
    float shininess = 32;
    vec4 color=vec4(1);
    std::string diffuseMap;
    std::string specularMap;
    std::string normalMap;
    std::string heightMap;

//    Texture diffuseMap;
//    Texture specularMap;
//    Texture normalMap;
//    Texture heightMap;
    Material() {}
};

#endif //OPENGL_CORE_MATERIAL_H
