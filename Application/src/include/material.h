//
// Created by ankit on 5/9/2021.
//
#include "core.h"
#include "Texture.h"

#ifndef OPENGL_CORE_MATERIAL_H
#define OPENGL_CORE_MATERIAL_H

struct Material {
    int id=-1;
    float AmbientStrength = 0.7;
    float DiffuseStrength = 1;
    float SpecularStrength = 1;
    vec3 specularColor = vec3(1);
    float shininess = 32;
    vec3 diffuseColor = vec3(1);
    std::string ambientMap;
    std::string diffuseMap;
    std::string specularMap;
    std::string normalMap;
    Material() {}
};

#endif //OPENGL_CORE_MATERIAL_H
