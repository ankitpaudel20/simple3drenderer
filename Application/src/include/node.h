#include "core.h"

struct node {
    //node* children=nullptr;
    //uint32_t nosChildren=0;
    std::map<std::string, node> children;
    std::vector<drawable<Vertex>> meshes;

    void delpos(const vec3 &delta) {

        for (auto &mesh : meshes)
            mesh.translation = glm::translate(mesh.translation, (glm::vec3)delta);

        for (auto &child : children)
            child.second.delpos(delta);
    }

    void setScale(const vec3 &scale) {

        for (auto &mesh : meshes)
            mesh.scaling = glm::scale(glm::mat4(1.0), (glm::vec3)scale);

        for (auto &child : children)
            child.second.setScale(scale);
    }

    void setRotation(const vec3 &rotate) {

        for (auto &mesh : meshes)
            mesh.rotation = {glm::rotate(glm::rotate(glm::rotate(glm::mat4(1.0), glm::radians(rotate.z), glm::vec3(0.0f, 0.0f, 1.0f)), glm::radians(rotate.y), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(rotate.x), glm::vec3(1.0f, 0.0f, 0.0f))};

        for (auto &child : children)
            child.second.setRotation(rotate);
    }
};