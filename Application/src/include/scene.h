//
// Created by ankit on 5/8/21.
//

#ifndef OPENGL_CORE_SCENE_H
#define OPENGL_CORE_SCENE_H

#include "core.h"
#include "drawable.h"
#include "camera.h"
#include "model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct node{
    node* children=nullptr;
    uint32_t nosChildren=0;
    //std::vector<node> children;
    std::vector<drawable<Vertex>> meshes;
};

struct scene{
    std::vector<node> nodes;
    vec3 ambientLight{1.0};
    std::vector<pointLight> pointLights;
    std::vector<dirLight> dirLights;

    drawable<Vertex> lightCube;

    camera cam;
    double deltatime = 0;

    scene(){
//        nodes.emplace_back(node());
    }

    void loadModel(const std::string &modelPath,const std::string& shaderName){
        node temp;
        temp.meshes=Model::loadModel(modelPath);
        for (auto &i:temp.meshes) {
            i.shader=shaderName;
        }
        nodes.push_back(temp);
    }

};

#endif //OPENGL_CORE_SCENE_H
