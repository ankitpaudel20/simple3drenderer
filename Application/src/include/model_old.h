
#pragma once

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "tinyobj/tiny_obj_loader.h"

#include <glm/gtc/matrix_transform.hpp>

#include <Shader.h>

#include "drawable.h"
#include "node.h"
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <sstream>
#include <string>
#include <vector>

static std::string directory;
static std::vector<std::string> textures_loaded;
static std::unordered_map<std::string, node> models_loaded;
static std::unordered_map<std::string, node *> models_loaded_path;
static std::unordered_map<std::string, Mesh> meshes_loaded;

namespace Model {

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
static std::string loadMaterialTexture(aiMaterial *mat, aiTextureType type, const std::string &typeName) {
    std::vector<std::string> textures;

    if (mat->GetTextureCount(type) > 1) {
        printf("there were multiple textures but only single was processed\n");
        /* code */
    }

    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        std::string relPath(str.C_Str());

#ifndef _WIN32
        size_t pos = 0;
        pos = relPath.find("\\");
        while (pos != std::string::npos) {
            relPath.replace(pos, 1, "/");
            pos = relPath.find("\\");
        }
#endif

        textures.push_back(directory + "/" + relPath);
    }

    if (!textures.empty())
        return textures[0];
    return std::string();
}

static drawable<Vertex> *processMesh(aiMesh *mesh, const aiScene *scene) {
    // data to fill
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<std::string> textures;

    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;
        // normals
        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
        }
        // texture coordinates
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoord = vec;
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.bitangent = vector;
        } else
            vertex.texCoord = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process materials
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    // 0. ambient maps
    std::string ambientMaps = loadMaterialTexture(material, aiTextureType_AMBIENT, "texture_ambient");
    textures.push_back(ambientMaps);
    // 1. diffuse maps
    std::string diffuseMaps = loadMaterialTexture(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.push_back(diffuseMaps);
    // 2. specular maps
    std::string specularMaps = loadMaterialTexture(material, aiTextureType_SPECULAR, "texture_specular");
    textures.push_back(specularMaps);
    // 3. normal maps
    std::string normalMaps = loadMaterialTexture(material, aiTextureType_HEIGHT, "texture_normal");
    textures.push_back(normalMaps);
    aiColor4D diffuse, specular;
    if (AI_SUCCESS != aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
        printf("diffuese color not found\n");
    if (AI_SUCCESS != aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular))
        printf("ambeinbt color not found\n");
    // return a mesh object created from the extracted mesh data

    std::string name(mesh->mName.C_Str());

    while (meshes_loaded.find(name) != meshes_loaded.end()) {
        name += "_copy";
    }
    meshes_loaded[name] = Mesh(vertices, indices, textures, name);
    meshes_loaded[name].material.diffuseColor = vec3(diffuse.r, diffuse.g, diffuse.b);
    meshes_loaded[name].material.specularColor = vec3(specular.r, specular.g, specular.b);
    return &meshes_loaded[name];
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
static void processNode(aiNode *node, const aiScene *scene, std::vector<drawable<Vertex> *> &meshes, int num) {
    int temp = num;
    // printf("processing %d node \n", temp);

    // process each mesh located at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        // the node object only contains indices to index the actual objects in the scene.
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        auto me = processMesh(mesh, scene);
        meshes.push_back(me);
        // printf("\t \t mesh %d processed\n", i);
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, meshes, ++num);
    }
    // printf("node %d processed \n", temp);
}

node *loadModel(std::string const &path, const std::string &shaderName, const std::string &name, bool flipUV = false) {

    if (models_loaded_path.find(path) != models_loaded_path.end()) {
        if (models_loaded_path[path] == &models_loaded[name]) {
            std::cout << "already loaded same module with same name and path os returning refrence to previous obj\n";
            DEBUG_BREAK;
        }
        auto previous_node = *models_loaded_path[path];
        node temp;
        for (auto &m : previous_node.meshes) {
            Mesh temp_mesh;
            temp_mesh.m_vertices = m->m_vertices;
            temp_mesh.m_indices = m->m_indices;
            temp_mesh.material = m->material;
            temp_mesh.shader = shaderName;
            auto name = m->name + "_copy";
            while (meshes_loaded.find(name) != meshes_loaded.end())
                name += "_copy";

            temp_mesh.name = name;
            meshes_loaded[temp_mesh.name] = std::move(temp_mesh);
            temp.meshes.push_back(&meshes_loaded[name]);
        }

        models_loaded[name] = std::move(temp);
        return &models_loaded[name];
    }

    if (models_loaded.find(name) != models_loaded.end()) {
        return &models_loaded[name];
    }
    // read file via ASSIMP
    Assimp::Importer importer;
    auto flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace;
    if (flipUV) {
        flags |= aiProcess_FlipUVs;
    }
    const aiScene *scene = importer.ReadFile(path, flags);
    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        DEBUG_BREAK;
        return nullptr;
    }
    // retrieve the directory path of the filepath
    directory = path.substr(0, path.find_last_of('/'));
    node temp;

    // std::vector<drawable<Vertex>> meshes;

    // process ASSIMP's root node recursively
    processNode(scene->mRootNode, scene, temp.meshes, 0);

    for (auto &i : temp.meshes) {
        i->shader = shaderName;
    }

    models_loaded[name] = std::move(temp);
    models_loaded_path[path] = &models_loaded[name];
    return &models_loaded[name];
}

} // namespace Model

namespace obj {

node *loadModel(std::string const &path, const std::string &shaderName, const std::string &name, bool flipUV = false) {

    if (models_loaded_path.find(path) != models_loaded_path.end()) {
        if (models_loaded_path[path] == &models_loaded[name]) {
            std::cout << "already loaded same module with same name and path os returning refrence to previous obj\n";
            DEBUG_BREAK;
        }
        auto previous_node = *models_loaded_path[path];
        node temp;
        for (auto &m : previous_node.meshes) {
            Mesh temp_mesh;
            temp_mesh.m_vertices = m->m_vertices;
            temp_mesh.m_indices = m->m_indices;
            temp_mesh.material = m->material;
            temp_mesh.shader = shaderName;
            auto name = m->name + "_copy";
            while (meshes_loaded.find(name) != meshes_loaded.end())
                name += "_copy";

            temp_mesh.name = name;
            meshes_loaded[temp_mesh.name] = std::move(temp_mesh);
            temp.meshes.push_back(&meshes_loaded[name]);
        }

        models_loaded[name] = std::move(temp);
        return &models_loaded[name];
    }

    if (models_loaded.find(name) != models_loaded.end()) {
        return &models_loaded[name];
    }

    // retrieve the directory path of the filepath
    directory = path.substr(0, path.find_last_of('/'));
    node temp;

    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = directory; // Path to material files
    reader_config.triangulate = true;          // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(path, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "[WARNING]: TinyObjReader: " << reader.Warning();
    }

    if (!reader.Error().empty()) {
        std::cout << "[Error]: TinyObjReader: " << reader.Error();
    }

    const tinyobj::attrib_t &attrib = reader.GetAttrib();
    auto &shapes = reader.GetShapes();
    auto &materials = reader.GetMaterials();

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {

        auto name = shapes[s].name;
        // Mesh *m1 = new Mesh();

        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<std::string> textures;

        size_t index_offset = 0;
        uint32_t current_index = 0;

        // Loop over faces(polygon)
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

            // // Optional:vertex colors
            // tinyobj::real_t red = attrib.colors[3 * size_t(idx.vertex_index) + 0];
            // tinyobj::real_t green = attrib.colors[3 * size_t(idx.vertex_index) + 1];
            // tinyobj::real_t blue = attrib.colors[3 * size_t(idx.vertex_index) + 2];

            // per-face material
            int current_material_id = shapes[s].mesh.material_ids[f];
            if ((current_material_id < 0) || (current_material_id >= static_cast<int>(materials.size()))) {
                // Invaid material ID. Use default material.
                current_material_id = materials.size() - 1; // Default material is added to the last item in `materials`.
            }
            if (textures.size() < 4) {
                textures.emplace_back(materials[current_material_id].ambient_texname);
                textures.emplace_back(materials[current_material_id].diffuse_texname);
                textures.emplace_back(materials[current_material_id].specular_texname);
                textures.emplace_back(materials[current_material_id].bump_texname);
            }
            if (current_material_id) {
                /* code */
            }

            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
            // indices = shapes[s].mesh.indices;

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                Vertex vertex;
                vertex.position = vec3(attrib.vertices[3 * size_t(idx.vertex_index) + 0], attrib.vertices[3 * size_t(idx.vertex_index) + 1], attrib.vertices[3 * size_t(idx.vertex_index) + 2]);

                //     // access to vertex
                // tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                // tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                // tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    vertex.normal = vec3(attrib.normals[3 * size_t(idx.vertex_index) + 0], attrib.vertices[3 * size_t(idx.vertex_index) + 1], attrib.vertices[3 * size_t(idx.vertex_index) + 2]);
                    // tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    // tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    // tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    vertex.texCoord = vec2(attrib.texcoords[2 * size_t(idx.texcoord_index) + 0], attrib.texcoords[2 * size_t(idx.texcoord_index) + 1]);
                    // tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    // tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                }

                vertices.push_back(vertex);
                indices.push_back(current_index++);
            }
            index_offset += fv;
        }

        while (meshes_loaded.find(name) != meshes_loaded.end()) {
            name += "_copy";
        }

        meshes_loaded[name] = Mesh(vertices, indices, textures, name);
        // meshes_loaded[name].material.diffuseColor = vec3(diffuse.r, diffuse.g, diffuse.b);
        // meshes_loaded[name].material.specularColor = vec3(specular.r, specular.g, specular.b);
        temp.meshes.push_back(&meshes_loaded[name]);
    }

    for (auto &i : temp.meshes) {
        i->shader = shaderName;
    }

    models_loaded[name] = std::move(temp);
    models_loaded_path[path] = &models_loaded[name];
    return &models_loaded[name];
}
} // namespace obj