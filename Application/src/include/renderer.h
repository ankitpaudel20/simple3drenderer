#include "core.h"
#include "buffer.h"
#include "Vertexarray.h"
#include "Texture.h"
#include "scene.h"
#include "callback.h"
#include "shapes.h"
#include "material.h"
#include "model.h"

struct entity {
    buffer<Vertex, GL_ARRAY_BUFFER> vbo;
    buffer<uint32_t, GL_ELEMENT_ARRAY_BUFFER> ibo;
    Vertexarray vao;
    drawable<Vertex> *model = nullptr;
};

class renderer {
    std::vector<entity> entities;

    void processNode(std::vector<node> &nodes) {
        for (auto &node:nodes) {
            for (auto &mesh: node.meshes) {
                entity temp;
                temp.vbo = buffer<Vertex, GL_ARRAY_BUFFER>(mesh.m_vertices);
                temp.ibo = buffer<uint32_t, GL_ELEMENT_ARRAY_BUFFER>(mesh.m_indices);
                temp.vao = Vertexarray(temp.vbo, temp.ibo);
                temp.model = &mesh;
                entities.push_back(temp);
            }
            processNode(node.children);
        }
    }

public:

    Shader *lightShader = nullptr;
    scene *currentScene;

    void init() {
        auto resPath = searchRes();
        lightShader = new Shader(resPath + "/shaders/lamp.vert", resPath + "/shaders/lamp.frag");
        currentScene->lightCube = drawable<Vertex>(cube::pos, cube::indices, lightShader);
        processNode(currentScene->nodes);
    }

    void Draw() {
        auto view = glm::lookAt(currentScene->cam.Camera_Position, currentScene->cam.Camera_Position + currentScene->cam.Camera_Facing_Direction * currentScene->cam.Camera_Target_distance, currentScene->cam.Camera_Up);

        auto projpersp = glm::perspective(glm::radians(currentScene->cam.FOV), aspect_ratio, currentScene->cam.nearPoint, currentScene->cam.farPoint);

        for (auto &entity:entities) {
            entity.vao.Bind();
            Shader *shader = entity.model->shader;
            shader->Bind();

            shader->SetUniform<glm::mat4 *>("model", entity.model->refreshModel());
            shader->SetUniform<glm::mat4 *>("view", &view);
            shader->SetUniform<glm::mat4 *>("view", &projpersp);
            shader->SetUniform<vec3>("ambientLight", currentScene->ambientLight);
            shader->SetUniform<vec3>("dirLight.direction", currentScene->dirLights[0].direction);
            shader->SetUniform<float>("dirLight.intensity", currentScene->dirLights[0].intensity);
            shader->SetUniform<vec3>("dirLight.diffusecolor", currentScene->dirLights[0].diffusecolor);
            shader->SetUniform<vec3>("dirLight.specularcolor", currentScene->dirLights[0].specularcolor);
            shader->SetUniform<vec3>("pointLights[0].position", currentScene->pointLights[0].position);
            shader->SetUniform<float>("pointLights[0].intensity", currentScene->pointLights[0].intensity);
            shader->SetUniform<vec3>("pointLights[0].diffusecolor", currentScene->pointLights[0].diffusecolor);
            shader->SetUniform<vec3>("pointLights[0].specularcolor", currentScene->pointLights[0].specularcolor);
            shader->SetUniform<float>("pointLights[0].constant", currentScene->pointLights[0].constant);
            shader->SetUniform<float>("pointLights[0].linear", currentScene->pointLights[0].linear);
            shader->SetUniform<float>("pointLights[0].quadratic", currentScene->pointLights[0].quadratic);
            shader->SetUniform<float>("material.shininess", entity.model->material.shininess);
            shader->SetUniform<vec3>("material.specularColor", entity.model->material.specularColor);
            shader->SetUniform<float>("material.ambientStrength", entity.model->material.AmbientStrength);
            shader->SetUniform<float>("material.diffuseStrength", entity.model->material.DiffuseStrength);
            shader->SetUniform<float>("material.specularStrength", entity.model->material.SpecularStrength);
            shader->SetUniform<int>("material.diffuseMap",0);
            shader->SetUniform<int>("material.specularMap",1);
        }

    }

    ~renderer() {
        delete lightShader;

        for (auto &i: entities) {
            i.vao.free();
            i.ibo.free();
            i.vbo.free();
            i.model->material.specularMap.free();
            i.model->material.diffuseMap.free();
        }
        buffer<uint32_t, GL_ELEMENT_ARRAY_BUFFER>::freeAll();
        buffer<Vertex, GL_ARRAY_BUFFER>::freeAll();
        Vertexarray::freeAll();
        Texture::freeAll();
    }

};
