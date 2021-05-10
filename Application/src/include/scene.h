//
// Created by ankit on 5/8/21.
//

#ifndef OPENGL_CORE_SCENE_H
#define OPENGL_CORE_SCENE_H

#include "core.h"
#include "drawable.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct node{
std::vector<node> children;
std::vector<drawable<Vertex>> meshes;
};

struct scene{
    std::vector<node> nodes;
    vec3 ambientLight{1.0};
    std::vector<pointLight> pointLights;
    std::vector<dirLight> dirLights;

    drawable<Vertex> lightCube;

    camera cam;

    void loadModel(const std::string &modelPath,const std::string shaderPath){
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }
};

#endif //OPENGL_CORE_SCENE_H
