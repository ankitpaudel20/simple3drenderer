#include "core.h"

struct node {
    //node* children=nullptr;
    //uint32_t nosChildren=0;
    std::map<std::string, node> children;
    std::vector<drawable<Vertex>> meshes;
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
