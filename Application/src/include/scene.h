//
// Created by ankit on 5/8/21.
//

#ifndef OPENGL_CORE_SCENE_H
#define OPENGL_CORE_SCENE_H

#include "camera.h"
#include "core.h"
#include "drawable.h"
#include "model.h"
#include "pointLight.h"
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct scene {
    std::unordered_map<std::string, node *> nodes;
    // std::vector<node> nodes;
    vec3 ambientLight{1.0};
    float ambientStrength = 0.5f;
    std::vector<pointLight> pointLights;
    dirLight dirLight;

    drawable<Vertex> lightCube;

    std::string skybox;

    camera cam;
    double deltatime = 0;

    scene() {
        //        nodes.emplace_back(node());
    }

    node *loadModel(const std::string &modelPath, const std::string &shaderName, const std::string &name, bool flipUV = false) {
        if (nodes.find(name) != nodes.end()) {
            return nodes[name];
        }
        nodes[name] = Model::loadModel(modelPath, shaderName, name, flipUV);
        return nodes[name];
    }

    node *loadModel_obj(const std::string &modelPath, const std::string &shaderName, const std::string &name, bool flipUV = false) {
        if (nodes.find(name) != nodes.end()) {
            return nodes[name];
        }
        nodes[name] = Model::loadModel_obj(modelPath, shaderName, name, flipUV);
        return nodes[name];
    }

    node *getModel(const std::string &name) {
        if (nodes.find(name) != nodes.end())
            return nodes[name];

        node *ret;
        for (auto &node : nodes)
            if ((ret = searchNode(node.second, name)) != nullptr)
                return ret;

        return nullptr;
    }

  private:
    node *searchNode(node *parent, const std::string &name) {
        if (parent->children.empty())
            return nullptr;

        if (parent->children.find(name) != parent->children.end())
            return &parent->children[name];

        for (auto &node : parent->children)
            return searchNode(&node.second, name);

        return nullptr;
    }
};

#endif // OPENGL_CORE_SCENE_H
