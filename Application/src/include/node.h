#include "core.h"

struct node {
    //node* children=nullptr;
    //uint32_t nosChildren=0;
    std::map<std::string, node> children;
    std::vector<drawable<Vertex> *> meshes;
    glm::mat4 matModel = glm::mat4(1.0);

    void delpos(const vec3 &delta) {

        for (auto &mesh : meshes)
            mesh->delpos(delta);

        for (auto &child : children)
            child.second.delpos(delta);
    }

    void setpos(const vec3 &position) {
        auto unit = glm::mat4();
        for (auto &mesh : meshes)
            mesh->setpos(position);

        for (auto &child : children)
            child.second.setpos(position);
    }

    void setScale(const vec3 &scale) {

        for (auto &mesh : meshes)
            mesh->setScale(scale);

        for (auto &child : children)
            child.second.setScale(scale);
    }

    void setRotation(const vec3 &rotate) {

        for (auto &mesh : meshes)
            mesh->setRotation(rotate);

        for (auto &child : children)
            child.second.setRotation(rotate);
    }

  private:
    glm::mat4 scaling = glm::mat4(1.0);
    glm::mat4 translation = glm::mat4(1.0);
    glm::mat4 rotation = glm::mat4(1.0);
};