#include <unordered_map>
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
    Shader* shader;
    Texture *diffuse,*specular;
};

class renderer {
    std::vector<entity> entities;

    void processNode(node* nodes,const uint32_t &n) {
        for (int i = 0; i <n ; ++i) {
            for (auto &mesh: nodes[i].meshes) {
                entity temp;
                temp.vbo = buffer<Vertex, GL_ARRAY_BUFFER>(mesh.m_vertices);
                temp.ibo = buffer<uint32_t, GL_ELEMENT_ARRAY_BUFFER>(mesh.m_indices);
                temp.vao = Vertexarray(temp.vbo, temp.ibo);
                temp.model = &mesh;
                if (shadersLoaded.find(mesh.shader)== shadersLoaded.end()){
                    shadersLoaded[mesh.shader]=Shader(resPath+"/shaders/"+mesh.shader,true);
                }
                if (texturesLoaded.find(mesh.material.diffuseMap)== texturesLoaded.end()){
                    texturesLoaded[mesh.material.diffuseMap]=Texture(mesh.material.diffuseMap);
                }
                if (texturesLoaded.find(mesh.material.specularMap)== texturesLoaded.end()){
                    texturesLoaded[mesh.material.specularMap]=Texture(mesh.material.specularMap);
                }

                temp.shader=&shadersLoaded[mesh.shader];
                temp.diffuse=&texturesLoaded[mesh.material.diffuseMap];
                temp.specular=&texturesLoaded[mesh.material.specularMap];
                entities.push_back(temp);
            }
            processNode(nodes[i].children,nodes[i].nosChildren);
        }

    }

public:

    static std::map<std::string,Texture> texturesLoaded;
    static std::map<std::string,Shader> shadersLoaded;

    scene *currentScene;
    std::string resPath;

    void init() {
        resPath = searchRes();
        if (!currentScene->pointLights.empty()){

            for (int i = 0; i < cube::pos.size(); ++i) {
                currentScene->lightCube.m_indices.push_back(i);
                currentScene->lightCube.m_vertices.emplace_back(cube::pos[i]);
            }
            currentScene->lightCube.shader="lamp";
            entity temp;
            temp.vbo = buffer<Vertex, GL_ARRAY_BUFFER>(currentScene->lightCube.m_vertices);
            temp.ibo = buffer<uint32_t, GL_ELEMENT_ARRAY_BUFFER>(currentScene->lightCube.m_indices);
            temp.vao = Vertexarray(temp.vbo, temp.ibo);
            temp.model = &currentScene->lightCube;
            temp.shader=&shadersLoaded[currentScene->lightCube.shader];
            entities.push_back(temp);
        }

        processNode(&currentScene->nodes[0],currentScene->nodes.size());
    }

    void Draw() {
        auto view = glm::lookAt(currentScene->cam.Camera_Position, currentScene->cam.Camera_Position + currentScene->cam.Camera_Facing_Direction * currentScene->cam.Camera_Target_distance, currentScene->cam.Camera_Up);

        auto projpersp = glm::perspective(glm::radians(currentScene->cam.FOV), aspect_ratio, currentScene->cam.nearPoint, currentScene->cam.farPoint);

        for (auto &entity:entities) {
            entity.vao.Bind();
            Shader *shader = entity.shader;
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

        for (auto &i: entities) {
            i.vao.free();
            i.ibo.free();
            i.vbo.free();
        }
        for (auto &i:shadersLoaded) {
            i.second.free();
        }
        for (auto & i:texturesLoaded) {
            i.second.free();
        }


        buffer<uint32_t, GL_ELEMENT_ARRAY_BUFFER>::freeAll();
        buffer<Vertex, GL_ARRAY_BUFFER>::freeAll();
        Vertexarray::freeAll();
        Texture::freeAll();
    }



};

std::map<std::string,Texture> renderer::texturesLoaded;
std::map<std::string,Shader> renderer::shadersLoaded;
