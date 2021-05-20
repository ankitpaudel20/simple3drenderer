#include "Texture.h"
#include "Vertexarray.h"
#include "buffer.h"
#include "callback.h"
#include "core.h"
#include "material.h"
#include "model.h"
#include "scene.h"
#include "shapes.h"
#include <unordered_map>

struct entity {
    buffer<Vertex, GL_ARRAY_BUFFER> vbo;
    buffer<uint32_t, GL_ELEMENT_ARRAY_BUFFER> ibo;
    Vertexarray vao;
    drawable<Vertex> *model = nullptr;
    Shader *shader;
    Texture *diffuse, *specular;
};

static std::map<std::string, Texture> texturesLoaded;
static std::map<std::string, Shader> shadersLoaded;

class renderer {
    std::vector<entity> entities;

    void processNode(std::map<std::string, node> &nodes) {
        for (auto &node : nodes) {
            for (auto &mesh : node.second.meshes) {
                entity temp;
                temp.vbo = buffer<Vertex, GL_ARRAY_BUFFER>(mesh.m_vertices);
                temp.ibo = buffer<uint32_t, GL_ELEMENT_ARRAY_BUFFER>(mesh.m_indices);
                temp.vao = Vertexarray(temp.vbo, temp.ibo);
                temp.model = &mesh;
                if (shadersLoaded.find(mesh.shader) == shadersLoaded.end())
                    shadersLoaded[mesh.shader] = Shader(resPath + "/shaders/" + mesh.shader, true);
                if (!mesh.material.diffuseMap.empty())
                    if (texturesLoaded.find(mesh.material.diffuseMap) == texturesLoaded.end())
                        texturesLoaded[mesh.material.diffuseMap] = Texture(mesh.material.diffuseMap);
                if (!mesh.material.specularMap.empty())
                    if (texturesLoaded.find(mesh.material.specularMap) == texturesLoaded.end())
                        texturesLoaded[mesh.material.specularMap] = Texture(mesh.material.specularMap);

                temp.shader = &shadersLoaded[mesh.shader];
                temp.diffuse = &texturesLoaded[mesh.material.diffuseMap];
                temp.specular = &texturesLoaded[mesh.material.specularMap];
                entities.push_back(temp);
            }
            processNode(node.second.children);
        }
    }

  public:
    scene *currentScene;
    std::string resPath;

    void init() {
        resPath = searchRes();

        /*if (!currentScene->pointLights.empty()) {

        for (int i = 0; i < cube::pos.size(); ++i) {
            currentScene->lightCube.m_indices.push_back(i);
            currentScene->lightCube.m_vertices.emplace_back(cube::pos[i]);
        }
        currentScene->lightCube.shader = "lamp";
        currentScene->lightCube.scaling = glm::scale(glm::mat4(1),
    glm::vec3(0.05, 0.05, 0.05)); currentScene->lightCube.translation =
    glm::translate(glm::mat4(1), glm::vec3(1)); if
    (shadersLoaded.find(currentScene->lightCube.shader) == shadersLoaded.end())
            shadersLoaded[currentScene->lightCube.shader] = Shader(resPath +
    "/shaders/" + currentScene->lightCube.shader, true); for (auto& i :
    currentScene->pointLights)
        {
            entity temp;
            temp.vbo = buffer<Vertex,
    GL_ARRAY_BUFFER>(currentScene->lightCube.m_vertices); temp.ibo =
    buffer<uint32_t,
    GL_ELEMENT_ARRAY_BUFFER>(currentScene->lightCube.m_indices); temp.vao =
    Vertexarray(temp.vbo, temp.ibo); temp.model = &currentScene->lightCube;

            temp.shader = &shadersLoaded[currentScene->lightCube.shader];
            entities.push_back(temp);
        }
    }*/

        processNode(currentScene->nodes);
    }

    void refreshNoPointLights() {
    }

    inline void clear() {
        (glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }

    inline void swapBuffers(GLFWwindow *win) { glfwSwapBuffers(win); }

    void Draw() {
        auto view = glm::lookAt(currentScene->cam.Camera_Position, currentScene->cam.Camera_Position + currentScene->cam.Camera_Facing_Direction * currentScene->cam.Camera_Target_distance, currentScene->cam.Camera_Up);

        auto projpersp = glm::perspective(glm::radians(currentScene->cam.FOV),
                                          aspect_ratio, currentScene->cam.nearPoint,
                                          currentScene->cam.farPoint);
        Shader *shader;

        for (auto &entity : entities) {
            entity.vao.Bind();
            shader = entity.shader;

            shader->Bind();
            shader->SetUniform<glm::mat4 *>("model", entity.model->refreshModel());
            shader->SetUniform<glm::mat4 *>("view", &view);
            shader->SetUniform<glm::mat4 *>("proj", &projpersp);

            if (entity.model->shader == "cube_final2") {

                shader->SetUniform<vec3>("ambientLight", currentScene->ambientLight);
                /*  shader->SetUniform<vec3>("dirLight.direction",
          currentScene->dirLights[0].direction);
          shader->SetUniform<float>("dirLight.intensity",
          currentScene->dirLights[0].intensity);
          shader->SetUniform<vec3>("dirLight.diffusecolor",
          currentScene->dirLights[0].diffusecolor);
          shader->SetUniform<vec3>("dirLight.specularcolor",
          currentScene->dirLights[0].specularcolor);*/
                shader->SetUniform<int>("activePointLights", currentScene->pointLights.size());
                shader->SetUniform<int>("doLightCalculations", entity.model->doLightCalculations);
                std::string lightString = "pointLights[";
                int i = 0;

                for (auto &light : currentScene->pointLights) {
                    lightString += std::to_string(i);
                    lightString.append("].position");
                    auto place = lightString.find_first_of(".") + 1;

                    shader->SetUniform<vec3>(lightString.c_str(), light.getpos());
                    lightString.erase(place);
                    lightString.append("intensity");
                    shader->SetUniform<float>(lightString.c_str(), light.intensity);
                    lightString.erase(place);
                    lightString.append("diffusecolor");
                    shader->SetUniform<vec3>(lightString.c_str(), light.getColor());
                    lightString.erase(place);
                    lightString.append("constant");
                    shader->SetUniform<float>(lightString.c_str(), light.constant);
                    lightString.erase(place);
                    lightString.append("linear");
                    shader->SetUniform<float>(lightString.c_str(), light.linear);
                    lightString.erase(place);
                    lightString.append("quadratic");
                    shader->SetUniform<float>(lightString.c_str(), light.quadratic);
                    i++;
                    /* code */
                }
                entity.diffuse->Bind(0);
                entity.specular->Bind(1);

                shader->SetUniform<float>("material.shininess", entity.model->material.shininess);
                shader->SetUniform<vec3>("material.specularColor", entity.model->material.specularColor);
                shader->SetUniform<vec3>("material.diffuseColor", entity.model->material.diffuseColor);
                shader->SetUniform<float>("material.ambientStrength", entity.model->material.AmbientStrength);
                shader->SetUniform<float>("material.diffuseStrength", entity.model->material.DiffuseStrength);
                shader->SetUniform<float>("material.specularStrength", entity.model->material.SpecularStrength);
                shader->SetUniform<int>("material.diffuseMap", 0);
                shader->SetUniform<int>("material.specularMap", 1);

                // shader->SetUniform<vec4>("material.color", entity.model->material.color);

                glDrawElements(GL_TRIANGLES, entity.ibo.m_count, GL_UNSIGNED_INT, nullptr);
            }
        }
    }

    ~renderer() {

        for (auto &i : entities) {
            i.vao.free();
            i.ibo.free();
            i.vbo.free();
        }
        for (auto &i : shadersLoaded) {
            i.second.free();
        }
        for (auto &i : texturesLoaded) {
            i.second.free();
        }

        buffer<uint32_t, GL_ELEMENT_ARRAY_BUFFER>::freeAll();
        buffer<Vertex, GL_ARRAY_BUFFER>::freeAll();
        Vertexarray::freeAll();
        Texture::freeAll();
    }
};
