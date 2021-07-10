#include <unordered_map>
#include <map>
#include <list>

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
    std::list<entity> entities;
    std::vector<uint32_t> depthfbo_pointlight, depthCubemap_pointlight;
    uint32_t depthfbo_dirlight, depthtexture_dirlight;
    const unsigned int POINT_SHADOW_WIDTH = 1920, POINT_SHADOW_HEIGHT = 1920;
    const unsigned int DIR_SHADOW_WIDTH = 1920, DIR_SHADOW_HEIGHT = 1920;
    float shadow_near_plane = 0.1f;
    float shadow_farplane = 125.0f;

    skyBox *skybox = nullptr;

    void processNode(node *n) {
        if (shadersLoaded.find(n->shaderName) == shadersLoaded.end())
            shadersLoaded[n->shaderName] = Shader(n->shaderName, resPath + "/shaders/" + n->shaderName, true);
        for (auto mesh : n->meshes) {
            entity temp;
            temp.vbo = buffer<Vertex, GL_ARRAY_BUFFER>(mesh->m_vertices);
            temp.ibo = buffer<uint32_t, GL_ELEMENT_ARRAY_BUFFER>(mesh->m_indices);
            temp.vao = Vertexarray(temp.vbo, temp.ibo);
            temp.mesh = mesh;
            if (shadersLoaded.find(mesh->shaderName) == shadersLoaded.end())
                shadersLoaded[mesh->shaderName] = Shader(mesh->shaderName, resPath + "/shaders/" + mesh->shaderName, true);
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

            temp.shader = &shadersLoaded[mesh->shaderName];
            temp.ambient = &texturesLoaded[mesh->material.ambientMap];
            temp.diffuse = &texturesLoaded[mesh->material.diffuseMap];
            temp.specular = &texturesLoaded[mesh->material.specularMap];
            temp.normal = &texturesLoaded[mesh->material.normalMap];
            entities.push_back(temp);
            mesh->gpuInstance = &entities.back();
        }
        for (auto &i : n->children) {
            processNode(i.second);
        }
    }

  public:
    scene *currentScene;
    std::string resPath;
    bool enable_shadows = true;
    bool enable_normals = true;

    void init() {
        resPath = searchRes();

        if (!currentScene->skybox.empty()) {
            skybox = new skyBox(currentScene->skybox);
            auto temp = Shader("skybox", resPath + "/shaders/skybox", true);
            shadersLoaded["skybox"] = std::move(temp);
        }
        for (auto &n : currentScene->nodes) {
            processNode(n.second);
        }
        for (size_t i = 0; i < currentScene->pointLights.size(); i++) {
            depthfbo_pointlight.push_back(0);
            glGenFramebuffers(1, &depthfbo_pointlight.back());
            depthCubemap_pointlight.push_back(0);
            glGenTextures(1, &depthCubemap_pointlight.back());
            glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap_pointlight.back());
            for (unsigned int i = 0; i < 6; ++i)
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, POINT_SHADOW_WIDTH, POINT_SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glBindFramebuffer(GL_FRAMEBUFFER, depthfbo_pointlight.back());
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap_pointlight.back(), 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        /* glGenFramebuffers(1, &depthfbo_dirlight);
        glGenTextures(1, &depthtexture_dirlight);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthtexture_dirlight);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, DIR_SHADOW_WIDTH, DIR_SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = {1.0, 1.0, 1.0, 1.0};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        glBindFramebuffer(GL_FRAMEBUFFER, depthfbo_dirlight);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthtexture_dirlight, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);*/

        shadersLoaded["point_light_shadow"] = Shader("point_light_shadow", resPath + "/shaders/point_shadows_depth", true);
    }

    inline void clear() { (glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); }

    inline void swapBuffers(GLFWwindow *win) { glfwSwapBuffers(win); }

    void recurseDrawModel(node *n, Shader *shader = nullptr, glm::mat4 model = glm::mat4(1)) {
        model = model * n->matModel;
        for (auto m : n->meshes) {
            shader->Bind();
            shader->SetUniform<const glm::mat4 &>("model", model * m->matModel);
            m->gpuInstance->vao.Bind();
            glDrawElements(GL_TRIANGLES, m->gpuInstance->ibo.m_count, GL_UNSIGNED_INT, nullptr);
        }
        for (auto child : n->children) {
            recurseDrawModel(child.second, shader, model);
        }
    }

    void renderDepthmap() {

        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)POINT_SHADOW_WIDTH / (float)POINT_SHADOW_HEIGHT, shadow_near_plane, shadow_farplane);
        // shadowTransforms.reserve(4);
        glViewport(0, 0, POINT_SHADOW_WIDTH, POINT_SHADOW_HEIGHT);
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
            glBindFramebuffer(GL_FRAMEBUFFER, depthfbo_pointlight[i]);
            glClear(GL_DEPTH_BUFFER_BIT);

            for (unsigned int i = 0; i < 6; ++i)
                shader.SetUniform<const glm::mat4 &>(("shadowMatrices[" + std::to_string(i) + "]").c_str(), shadowTransforms[i]);
            shader.SetUniform<float>("far_plane", shadow_farplane);
            shader.SetUniform<vec3>("lightPos", lights[i].getpos());

            for (auto n : currentScene->nodes) {
                recurseDrawModel(n.second, &shader);
            }

            shadowTransforms.clear();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, winWIDTH, winHEIGHT);
    }

    float left = 0, right = 100, top = 0, bottom = 500;
    float bias = 0.0001;
    void recurseDraw(node *n, const glm::mat4 &viewProj, Shader *shader = nullptr, glm::mat4 model = glm::mat4(1)) {
        model = model * n->matModel;
        if (shader == nullptr) {
            shader = &shadersLoaded[n->shaderName];
            if (n->shaderName == "cube_final2") {
                shader->Bind();
                shader->SetUniform<float>("bias", bias);

                shader->SetUniform<const glm::mat4 &>("viewProj", viewProj);
                shader->SetUniform<int>("enable_normals", enable_normals);

                shader->SetUniform<vec3>("camPos", currentScene->cam.Camera_Position);
                shader->SetUniform<vec3>("ambientLight", currentScene->ambientLight);
                shader->SetUniform<float>("ambientStrength", currentScene->ambientStrength);

                shader->SetUniform<vec3>("dirLight.direction", currentScene->dir_light.direction);
                shader->SetUniform<float>("dirLight.intensity", currentScene->dir_light.intensity);
                shader->SetUniform<vec3>("dirLight.color", currentScene->dir_light.color);

                shader->SetUniform<int>("activePointLights", currentScene->pointLights.size());

                std::string lightString = "pointLights[";
                int i = 0;
                for (auto &light : currentScene->pointLights) {
                    lightString += std::to_string(i);
                    lightString.append("].");
                    auto place = lightString.find_first_of(".") + 1;
                    lightString.append("position");
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
                    lightString.erase(place);
                    lightString.append("radius");
                    shader->SetUniform<float>(lightString.c_str(), light.radius);
                    //hardcoded 5 for now because we know there are 4 maps and it starts from 1
                    glActiveTexture(GL_TEXTURE0 + 5 + i);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap_pointlight[i]);
                    lightString.erase(place);
                    lightString.append("depthMap");
                    shader->SetUniform<int>(lightString.c_str(), 5 + i);
                    lightString.erase(place - 3);
                    i++;
                }

                shader->SetUniform<float>("shadow_farplane", shadow_farplane);
                shader->SetUniform<int>("enable_shadows", enable_shadows);
            }
        }

        for (auto m : n->meshes) {
            shader->Bind();
            shader->SetUniform<const glm::mat4 &>("model", model * m->matModel);
            if (shader->name == "cube_final2") {
                uint32_t sampler_counter = 1;
                shader->SetUniform<float>("material.shininess", m->material.shininess);
                shader->SetUniform<vec3>("material.specularColor", m->material.specularColor);
                shader->SetUniform<vec3>("material.diffuseColor", m->material.diffuseColor);
                shader->SetUniform<float>("material.ambientStrength", m->material.AmbientStrength);
                shader->SetUniform<float>("material.diffuseStrength", m->material.DiffuseStrength);
                shader->SetUniform<float>("material.specularStrength", m->material.SpecularStrength);
                m->gpuInstance->ambient->Bind(sampler_counter);
                shader->SetUniform<int>("material.ambientMap", sampler_counter++);
                m->gpuInstance->diffuse->Bind(sampler_counter);
                shader->SetUniform<int>("material.diffuseMap", sampler_counter++);
                m->gpuInstance->specular->Bind(sampler_counter);
                shader->SetUniform<int>("material.specularMap", sampler_counter++);
                m->gpuInstance->normal->Bind(sampler_counter);
                shader->SetUniform<int>("material.normalMap", sampler_counter++);
                shader->SetUniform<int>("doLightCalculations", m->doLightCalculations);
                m->gpuInstance->vao.Bind();
                glDrawElements(GL_TRIANGLES, m->gpuInstance->ibo.m_count, GL_UNSIGNED_INT, nullptr);
            }
        }
        for (auto &i : n->children) {
            recurseDraw(i.second, viewProj, shader, model);
        }
    }

    void Draw() {
        clear();
        if (enable_shadows) {
            renderDepthmap();
        }

        auto view = glm::lookAt(currentScene->cam.Camera_Position, currentScene->cam.Camera_Position + currentScene->cam.Camera_Facing_Direction * currentScene->cam.Camera_Target_distance, currentScene->cam.Camera_Up);

        auto projpersp = glm::perspective(glm::radians(currentScene->cam.FOV), aspect_ratio, currentScene->cam.nearPoint, currentScene->cam.farPoint);
        // auto projpersp = glm::ortho(left, -left, top, -top, currentScene->cam.nearPoint, currentScene->cam.farPoint);
        auto viewProj = projpersp * view;

        for (auto n : currentScene->nodes) {
            recurseDraw(n.second, viewProj);
        }

        if (skybox) {
            skybox->draw(&shadersLoaded["skybox"], view, glm::perspective(glm::radians(currentScene->cam.FOV), aspect_ratio, currentScene->cam.nearPoint, currentScene->cam.farPoint));
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
        for (auto &i : depthfbo_pointlight) {
            glDeleteFramebuffers(1, &i);
        }
        for (auto &i : depthCubemap_pointlight) {
            glDeleteTextures(1, &i);
        }

        delete skybox;

        buffer<uint32_t, GL_ELEMENT_ARRAY_BUFFER>::freeAll();
        buffer<Vertex, GL_ARRAY_BUFFER>::freeAll();
        Vertexarray::freeAll();
        Texture::freeAll();
    }
};
