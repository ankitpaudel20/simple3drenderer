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
                shadersLoaded[mesh.shader]=Shader(resPath+"/shaders/"+mesh.shader,true);
                temp.shader=&shadersLoaded[mesh.shader];
                entities.push_back(temp);
            }
            processNode(node.children);
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

        processNode(currentScene->nodes);
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

    static std::string directory;

    bool gammaCorrection;

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    static std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName) {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);
            //std::string path = str.C_Str();
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            std::string relPath(str.C_Str());
#ifndef _WIN32
            size_t pos=0;
            pos=relPath.find("\\");
            while (pos!=std::string::npos)
            {
                relPath.replace(pos,1,"/");
                pos=relPath.find("\\");
            }
#endif

            for (auto &j:textures_loaded) {
                if (j.getPath() == directory + "/" + relPath) {
                    textures.push_back(j);

                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }


            if (!skip) {   // if texture hasn't been loaded already, load it
                Texture texture(Model::directory + "/" + relPath);
                texture.type = typeName;
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures;
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    static Texture loadMaterialTexture(aiMaterial *mat, aiTextureType type, const std::string &typeName) {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);
            //std::string path = str.C_Str();
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            std::string relPath(str.C_Str());

#ifndef _WIN32
            size_t pos=0;
            pos=relPath.find("\\");
            while (pos!=std::string::npos)
            {
                relPath.replace(pos,1,"/");
                pos=relPath.find("\\");
            }
#endif

            for (auto &j:textures_loaded) {
                if (j.getPath() == Model::directory + "/" + relPath) {
                    textures.push_back(j);

                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }


            if (!skip) {   // if texture hasn't been loaded already, load it
                Texture texture(Model::directory + "/" + relPath);
                texture.type = typeName;
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        if (!textures.empty())
            return textures[0];
        return Texture();
    }

    static drawable<Vertex> processMesh(aiMesh *mesh, const aiScene *scene) {
        // data to fill
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

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

//		// 1. diffuse maps
//		std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
//		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
//		// 2. specular maps
//		std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
//		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
//		// 3. normal maps
//		std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_normal");
//		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
//		// 4. height maps
//		std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_height");
//		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        // 1. diffuse maps
        Texture diffuseMaps = loadMaterialTexture(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.push_back(diffuseMaps);
        // 2. specular maps
        Texture specularMaps = loadMaterialTexture(material, aiTextureType_SPECULAR, "texture_specular");
        textures.push_back(specularMaps);
        // 3. normal maps
        Texture normalMaps = loadMaterialTexture(material, aiTextureType_AMBIENT, "texture_normal");
        textures.push_back(normalMaps);
        // 4. height maps
        Texture heightMaps = loadMaterialTexture(material, aiTextureType_HEIGHT, "texture_height");
        textures.push_back(heightMaps);

        // return a mesh object created from the extracted mesh data
        drawable<Vertex> toreturn(vertices, indices, textures);
        return toreturn;
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    static void processNode(aiNode *node, const aiScene *scene, std::vector<drawable<Vertex>> &meshes, int num) {
        int temp = num;
        printf("processing %d node \n", temp);

        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            // the node object only contains indices to index the actual objects in the scene.
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            auto me = processMesh(mesh, scene);
            meshes.push_back(std::move(me));
            printf("\t \t mesh %d processed\n", i);
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene, meshes, ++num);
        }
        printf("node %d processed \n", temp);
    }



// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    std::vector<drawable<Vertex>> loadModel(std::string const &path) {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            DEBUG_BREAK;
            return std::vector<drawable<Vertex>>();
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));
        std::vector<drawable<Vertex>> meshes;

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene, meshes, 0);
        return meshes;
    }

};
