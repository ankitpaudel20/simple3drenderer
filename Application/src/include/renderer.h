#include <unordered_map>

#include "Texture.h"
#include "Vertexarray.h"
#include "buffer.h"
#include "callback.h"
#include "core.h"
#include "material.h"
#include "model.h"
#include "scene.h"
#include "shapes.h"
#include "skybox.h"

struct entity {
    buffer<Vertex, GL_ARRAY_BUFFER> vbo;
    buffer<uint32_t, GL_ELEMENT_ARRAY_BUFFER> ibo;
    Vertexarray vao;
    drawable<Vertex> *mesh = nullptr;
    Shader *shader;
    Texture *ambient, *diffuse, *specular, *normal;
};

static std::unordered_map<std::string, Texture> texturesLoaded;
static std::unordered_map<std::string, Shader> shadersLoaded;

class renderer {
    std::vector<entity> entities;
    std::vector<uint32_t> depthfbo, depthCubemap;
    const unsigned int SHADOW_WIDTH = 1920, SHADOW_HEIGHT = 1920;
    float shadow_near_plane = 0.1f;
    float shadow_farplane = 125.0f;

    skyBox *skybox = nullptr;

    void processNode(node *node) {
        for (auto mesh : node->meshes) {
            entity temp;
            temp.vbo = buffer<Vertex, GL_ARRAY_BUFFER>(mesh->m_vertices);
            temp.ibo = buffer<uint32_t, GL_ELEMENT_ARRAY_BUFFER>(mesh->m_indices);
            temp.vao = Vertexarray(temp.vbo, temp.ibo);
            temp.mesh = mesh;
            if (shadersLoaded.find(mesh->shader) == shadersLoaded.end())
                shadersLoaded[mesh->shader] = Shader(resPath + "/shaders/" + mesh->shader, true);
            if (!mesh->material.ambientMap.empty())
                if (texturesLoaded.find(mesh->material.ambientMap) == texturesLoaded.end())
                    texturesLoaded[mesh->material.ambientMap] = Texture(mesh->material.ambientMap);
            if (!mesh->material.diffuseMap.empty())
                if (texturesLoaded.find(mesh->material.diffuseMap) == texturesLoaded.end())
                    texturesLoaded[mesh->material.diffuseMap] = Texture(mesh->material.diffuseMap);
            if (!mesh->material.specularMap.empty())
                if (texturesLoaded.find(mesh->material.specularMap) == texturesLoaded.end())
                    texturesLoaded[mesh->material.specularMap] = Texture(mesh->material.specularMap);
            if (!mesh->material.normalMap.empty())
                if (texturesLoaded.find(mesh->material.normalMap) == texturesLoaded.end())
                    texturesLoaded[mesh->material.normalMap] = Texture(mesh->material.normalMap);

            temp.shader = &shadersLoaded[mesh->shader];
            temp.ambient = &texturesLoaded[mesh->material.ambientMap];
            temp.diffuse = &texturesLoaded[mesh->material.diffuseMap];
            temp.specular = &texturesLoaded[mesh->material.specularMap];
            temp.normal = &texturesLoaded[mesh->material.normalMap];
            entities.push_back(temp);
        }

        for (auto &i : node->children) {
            processNode(&i.second);
        }
    }

  public:
    scene *currentScene;
    std::string resPath;
    bool enable_shadows = true;

    void init() {
        resPath = searchRes();

        if (!currentScene->skybox.empty()) {
            skybox = new skyBox(currentScene->skybox);
            shadersLoaded["skybox"] = Shader(resPath + "/shaders/skybox", true);
        }
        for (auto &n : currentScene->nodes) {
            processNode(n.second);
        }
        for (size_t i = 0; i < currentScene->pointLights.size(); i++) {
            depthfbo.push_back(0);
            glGenFramebuffers(1, &depthfbo.back());
            depthCubemap.push_back(0);
            glGenTextures(1, &depthCubemap.back());
            glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap.back());
            for (unsigned int i = 0; i < 6; ++i)
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glBindFramebuffer(GL_FRAMEBUFFER, depthfbo.back());
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap.back(), 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            /* code */
        }
        shadersLoaded["point_light_shadow"] = Shader(resPath + "/shaders/point_shadows_depth", true);
    }

    inline void clear() { (glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); }

    inline void swapBuffers(GLFWwindow *win) { glfwSwapBuffers(win); }

    void renderDepthmap() {

        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, shadow_near_plane, shadow_farplane);
        // shadowTransforms.reserve(4);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        auto &lights = currentScene->pointLights;
        auto &shader = shadersLoaded["point_light_shadow"];
        shader.Bind();
        std::vector<glm::mat4> shadowTransforms;
        for (size_t i = 0; i < lights.size(); i++) {

            shadowTransforms.push_back(shadowProj * glm::lookAt((glm::vec3)lights[i].getpos(), (glm::vec3)(lights[i].getpos() + vec3(1.0f, 0.0f, 0.0f)), glm::vec3(0.0f, -1.0f, 0.0f)));
            shadowTransforms.push_back(shadowProj * glm::lookAt((glm::vec3)lights[i].getpos(), (glm::vec3)(lights[i].getpos() + vec3(-1.0f, 0.0f, 0.0f)), glm::vec3(0.0f, -1.0f, 0.0f)));
            shadowTransforms.push_back(shadowProj * glm::lookAt((glm::vec3)lights[i].getpos(), (glm::vec3)(lights[i].getpos() + vec3(0.0f, 1.0f, 0.0f)), glm::vec3(0.0f, 0.0f, 1.0f)));
            shadowTransforms.push_back(shadowProj * glm::lookAt((glm::vec3)lights[i].getpos(), (glm::vec3)(lights[i].getpos() + vec3(0.0f, -1.0f, 0.0f)), glm::vec3(0.0f, 0.0f, -1.0f)));
            shadowTransforms.push_back(shadowProj * glm::lookAt((glm::vec3)lights[i].getpos(), (glm::vec3)(lights[i].getpos() + vec3(0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, -1.0f, 0.0f)));
            shadowTransforms.push_back(shadowProj * glm::lookAt((glm::vec3)lights[i].getpos(), (glm::vec3)(lights[i].getpos() + vec3(0.0f, 0.0f, -1.0f)), glm::vec3(0.0f, -1.0f, 0.0f)));
            glBindFramebuffer(GL_FRAMEBUFFER, depthfbo[i]);
            glClear(GL_DEPTH_BUFFER_BIT);

            for (unsigned int i = 0; i < 6; ++i)
                shader.SetUniform<glm::mat4 *>(("shadowMatrices[" + std::to_string(i) + "]").c_str(), &shadowTransforms[i]);
            shader.SetUniform<float>("far_plane", shadow_farplane);
            shader.SetUniform<vec3>("lightPos", lights[i].getpos());

            for (auto &entity : entities) {
                if (entity.mesh->draw) {
                    entity.vao.Bind();
                    shader.SetUniform<glm::mat4 *>("model", &entity.mesh->matModel);
                    glDrawElements(GL_TRIANGLES, entity.ibo.m_count, GL_UNSIGNED_INT, nullptr);
                }
            }
            shadowTransforms.clear();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, winWIDTH, winHEIGHT);
    }

    void Draw() {
        clear();
        if (enable_shadows) {
            renderDepthmap();
        }

        auto view = glm::lookAt(currentScene->cam.Camera_Position, currentScene->cam.Camera_Position + currentScene->cam.Camera_Facing_Direction * currentScene->cam.Camera_Target_distance, currentScene->cam.Camera_Up);

        auto projpersp = glm::perspective(glm::radians(currentScene->cam.FOV), aspect_ratio, currentScene->cam.nearPoint, currentScene->cam.farPoint);
        auto viewProj = projpersp * view;
        Shader *shader;

        for (auto &entity : entities) {
            if (entity.mesh->draw) {

                shader = entity.shader;

                shader->Bind();
                shader->SetUniform<glm::mat4 *>("model", &entity.mesh->matModel);
                shader->SetUniform<glm::mat4 *>("viewProj", &viewProj);

                if (entity.mesh->shader == "cube_final2") {
                    shader->SetUniform<vec3>("camPos", currentScene->cam.Camera_Position);
                    shader->SetUniform<vec3>("ambientLight", currentScene->ambientLight);
                    uint32_t sampler_counter = 1;

                    shader->SetUniform<float>("material.shininess", entity.mesh->material.shininess);
                    shader->SetUniform<vec3>("material.specularColor", entity.mesh->material.specularColor);
                    shader->SetUniform<vec3>("material.diffuseColor", entity.mesh->material.diffuseColor);
                    shader->SetUniform<float>("material.ambientStrength", entity.mesh->material.AmbientStrength);
                    shader->SetUniform<float>("material.diffuseStrength", entity.mesh->material.DiffuseStrength);
                    shader->SetUniform<float>("material.specularStrength", entity.mesh->material.SpecularStrength);
                    entity.ambient->Bind(sampler_counter);
                    shader->SetUniform<int>("material.ambientMap", sampler_counter++);
                    entity.diffuse->Bind(sampler_counter);
                    shader->SetUniform<int>("material.diffuseMap", sampler_counter++);
                    entity.specular->Bind(sampler_counter);
                    shader->SetUniform<int>("material.specularMap", sampler_counter++);
                    entity.normal->Bind(sampler_counter);
                    shader->SetUniform<int>("material.normalMap", sampler_counter++);

                    shader->SetUniform<int>("doLightCalculations", entity.mesh->doLightCalculations);
                    if (entity.mesh->doLightCalculations) {
                        /* shader->SetUniform<vec3>("dirLight.direction", currentScene->dirLights[0].direction);
                        shader->SetUniform<float>("dirLight.intensity", currentScene->dirLights[0].intensity);
                        shader->SetUniform<vec3>("dirLight.color", currentScene->dirLights[0].color);*/

                        shader->SetUniform<int>("activePointLights", currentScene->pointLights.size());

                        std::string lightString = "pointLights[";
                        int i = 1;

                        for (auto &light : currentScene->pointLights) {
                            lightString += std::to_string(i);
                            lightString.append("].position");
                            auto place = lightString.find_first_of(".") + 1;

                            shader->SetUniform<vec3>(lightString.c_str(), light.getpos());
                            lightString.erase(place);
                            lightString.append("intensity");
                            shader->SetUniform<float>(lightString.c_str(), light.intensity);
                            lightString.erase(place);
                            lightString.append("diffuseColor");
                            shader->SetUniform<vec3>(lightString.c_str(), light.get_diffuse_color());
                            lightString.erase(place);
                            lightString.append("ambientColor");
                            shader->SetUniform<vec3>(lightString.c_str(), light.get_ambient_color());
                            lightString.erase(place);
                            lightString.append("constant");
                            shader->SetUniform<float>(lightString.c_str(), light.constant);
                            lightString.erase(place);
                            lightString.append("linear");
                            shader->SetUniform<float>(lightString.c_str(), light.linear);
                            lightString.erase(place);
                            lightString.append("quadratic");
                            shader->SetUniform<float>(lightString.c_str(), light.quadratic);

                            glActiveTexture(GL_TEXTURE0 + sampler_counter);
                            glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap[i]);
                            lightString.erase(place);
                            lightString.append("depthMap");
                            shader->SetUniform<int>(lightString.c_str(), sampler_counter++);
                            i++;
                        }

                        shader->SetUniform<float>("shadow_farplane", shadow_farplane);
                        shader->SetUniform<int>("enable_shadows", enable_shadows);
                    }
                    entity.vao.Bind();
                    glDrawElements(GL_TRIANGLES, entity.ibo.m_count, GL_UNSIGNED_INT, nullptr);
                }
            }
        }
        if (skybox) {
            skybox->draw(&shadersLoaded["skybox"], view, projpersp);
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
        for (auto &i : depthfbo) {
            glDeleteFramebuffers(1, &i);
        }
        for (auto &i : depthCubemap) {
            glDeleteTextures(1, &i);
        }

        delete skybox;

        buffer<uint32_t, GL_ELEMENT_ARRAY_BUFFER>::freeAll();
        buffer<Vertex, GL_ARRAY_BUFFER>::freeAll();
        Vertexarray::freeAll();
        Texture::freeAll();
    }
};
