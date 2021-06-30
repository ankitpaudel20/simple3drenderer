#include "core.h"

struct pointLight {
  private:
    vec3 position;
    vec3 diffuseColor;
    vec3 ambientColor = vec3(1);
    node *model = nullptr;

    inline void refreshModelMatrix() {
        for (auto &mesh : model->meshes) {
            // auto iden = glm::mat4(1);
            mesh->setpos(position);
        }
    }

  public:
    float intensity;
    float radius = 11;
    float dropoffRadius = 1;

    // float constant = 1;
    // float linear = 0.09;
    // float quadratic = 0.032;
    float constant = 0.85;
    float linear = 1.0;
    float quadratic = 0.06;

    pointLight(const vec3 &pos, const float &intensity, const vec3 &diffcol = vec3(1)) : position(pos), diffuseColor(diffcol), intensity(intensity) {}

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
        for (auto mesh : model->meshes) {
            mesh->material.diffuseColor = diffuseColor;
        }

        refreshModelMatrix();
    }

    void setdiffColor(const vec3 &color) {
        diffuseColor = color;
        if (model) {
            for (auto &mesh : model->meshes) {
                mesh->material.diffuseColor = color;
                mesh->material.specularColor = color;
            }
        }
    }

    void setColor(const vec3 &color) {
        ambientColor = color;
        setdiffColor(color);
    }

    [[nodiscard]] inline vec3 &getpos() { return position; }
    [[nodiscard]] inline vec3 &get_diffuse_color() { return diffuseColor; }
    [[nodiscard]] inline vec3 &get_ambient_color() { return ambientColor; }
    [[nodiscard]] inline node *&getModel() { return model; }
};
