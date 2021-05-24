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

struct pointLight {
  private:
    vec3 position;
    vec3 diffuseColor;
    node *model;

    inline void refreshModelMatrix() {
        for (auto &mesh : model->meshes) {
            mesh.translation = glm::translate(glm::mat4(1), (glm::vec3)position);
        }
    }

  public:
    float intensity;

    float constant = 1;
    float linear = 0.09;
    float quadratic = 0.032;

    pointLight(const vec3 &pos, const float &intensity, const vec3 &diffcol = 1) : position(pos), diffuseColor(diffcol), intensity(intensity) {}

    void delpos(const vec3 &delta) {
        position += delta;
        if (model) {
            refreshModelMatrix();
        }
    }

    void setpos(const vec3 &newPos) {
        position = newPos;
        if (model) {
            refreshModelMatrix();
        }
    }

    void setmodel(node *model) {
        this->model = model;
        refreshModelMatrix();
    }

    void setColor(const vec3 &color) {
        diffuseColor = color;
        if (model) {
            for (auto &mesh : model->meshes) {
                mesh.material.diffuseColor = color;
            }
        }
    }

    [[nodiscard]] inline vec3 &getpos() { return position; }
    [[nodiscard]] inline vec3 &getColor() { return diffuseColor; }
    [[nodiscard]] inline node *&getModel() { return model; }
};
