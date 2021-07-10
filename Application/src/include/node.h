#include "core.h"

struct node {
    //node* children=nullptr;
    //uint32_t nosChildren=0;
    std::map<std::string, node *> children;
    std::vector<drawable<Vertex> *> meshes;
    glm::mat4 matModel = glm::mat4(1.0);
    std::string shaderName;

    void delpos(const vec3 &delta) {
        translation = glm::translate(translation, (glm::vec3)(delta));
        refreshModel();
    }

    void setpos(const vec3 &position) {
        translation = glm::translate(glm::mat4(1), (glm::vec3)(position));
        refreshModel();
    }

    void setScale(const vec3 &scale) {
        scaling = glm::scale(scaling, (glm::vec3)(scale));
        refreshModel();
    }

    void setRotation(float angle, const vec3 &axis) {
        rotation = glm::rotate(rotation, glm::radians(angle), (glm::vec3)axis);
        refreshModel();
    }

    const glm::mat4 &refreshModel() {
        matModel = translation * rotation * scaling;
        return matModel;
    }

  private:
    glm::mat4 scaling = glm::mat4(1.0);
    glm::mat4 translation = glm::mat4(1.0);
    glm::mat4 rotation = glm::mat4(1.0);
};