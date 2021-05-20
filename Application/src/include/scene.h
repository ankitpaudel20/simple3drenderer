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
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct scene {
    std::map<std::string, node> nodes;
    // std::vector<node> nodes;
    vec3 ambientLight{1.0};
    std::vector<pointLight> pointLights;
    std::vector<dirLight> dirLights;

    drawable<Vertex> lightCube;

    camera cam;
    double deltatime = 0;

    scene() {
        //        nodes.emplace_back(node());
    }

    void loadModel(const std::string &modelPath, const std::string &shaderName, const std::string &name) {
        node temp;
        temp.meshes = Model::loadModel(modelPath);
        for (auto &i : temp.meshes) {
            i.shader = shaderName;
        }
        nodes[name] = temp;
    }

    node *getModel(const std::string &name) {
        if (nodes.find(name) != nodes.end())
            return &nodes[name];

        node *ret;
        for (auto &node : nodes)
            if ((ret = searchNode(&node.second, name)) != nullptr)
                return ret;

        return nullptr;
    }

  private:
    static node *searchNode(node *parent, const std::string &name) {
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
