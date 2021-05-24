#pragma once

#include <utility>

#include "core.h"
//#include "Vertexarray.h"
//#include "buffer.h"
//#include "Shader.h"
#include "material.h"

uint32_t noDrawCalls = 0;

inline vec3 operator*(glm::mat4 mat, vec3 vec) {
    glm::vec4 temp(vec.x, vec.y, vec.z, 1);
    temp = mat * temp;
    return vec3(temp.x, temp.y, temp.z);
}

struct entity;

template <class T>
struct drawable {
    std::vector<T> m_vertices;
    std::vector<uint32_t> m_indices;
    uint32_t m_primitve = GL_TRIANGLES;
    Material material;
    std::string shader;
    entity *gpuInstance = nullptr;
    uint32_t entity_index;
    bool doLightCalculations = true;

    glm::mat4 scaling = glm::mat4(1.0);
    glm::mat4 translation = glm::mat4(1.0);
    glm::mat4 rotation = glm::mat4(1.0);
    glm::mat4 matModel = glm::mat4(1.0);

    drawable() = default;

    drawable(const std::vector<T> &vertices, std::vector<unsigned> indices, std::vector<std::string> tex) : m_vertices(vertices), m_indices(std::move(indices)) {
        material.ambientMap = tex[0];
        material.diffuseMap = tex[1];
        material.specularMap = tex[2];
        material.normalMap = tex[3];
    }

    glm::mat4 *refreshModel() {
        matModel = translation * rotation * scaling;
        return &matModel;
    }
};