#include "stb_image/stb_image.h"
#include <string>

#include "core.h"
#include "Vertexarray.h"
#include "buffer.h"

class cubeMap {
    std::string m_filepath;
    int m_width, m_height, m_BPP;

  public:
    unsigned ID = 0;

    uint32_t getID() const { return ID; }

    std::string getPath() { return m_filepath; }

    cubeMap() = default;

    cubeMap(const std::string &filepath) : m_filepath(filepath) {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

        unsigned char *m_LocalBuffer;

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        std::vector<std::string> faces{
            pathDelemeter + "right.jpg",
            pathDelemeter + "left.jpg",
            pathDelemeter + "top.jpg",
            pathDelemeter + "bottom.jpg",
            pathDelemeter + "front.jpg",
            pathDelemeter + "back.jpg"};

        stbi_set_flip_vertically_on_load(0);
        for (unsigned int i = 0; i < faces.size(); i++) {
            auto fullpath = m_filepath + faces[i];
            m_LocalBuffer = stbi_load(fullpath.c_str(), &m_width, &m_height, &m_BPP, 0);

            if (!m_LocalBuffer) {
                std::cout << "skybox texture file unable to load" << std::endl;
                assert(false);
            }

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_LocalBuffer);
            stbi_image_free(m_LocalBuffer);
        }

        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        Unbind();
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    static void Unbind(unsigned slot = 0) {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void bind() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    }

    ~cubeMap() {
        if (ID)
            std::cout << "cubemap Texture freed\n";
        glDeleteTextures(1, &ID);
        ID = 0;
    }
};

class skyBox {
    cubeMap map;
    uint32_t m_vao = 0, m_vbo = 0;

    std::vector<float> vertices = {
        // positions
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f};

  public:
    skyBox() = default;

    skyBox(const std::string &filepath) : map(filepath) {
        glGenBuffers(1, &m_vbo);
        glGenVertexArrays(1, &m_vao);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void draw(Shader *shader, const glm::mat3 &view, const glm::mat4 &projection, uint32_t testCubemap = 0) {
        glm::mat4 mView(view);  // remove translation from the view matrix
        glDepthFunc(GL_LEQUAL); // change depth function so depth test passes when values are equal to depth buffer's content
        if (testCubemap == 0) {
            map.bind();
        } else {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, testCubemap);
        }

        shader->Bind();
        shader->SetUniform<const glm::mat4 &>("view", mView);
        shader->SetUniform<const glm::mat4 &>("proj", projection);
        shader->SetUniform<int>("skybox", 0);

        // skybox cube
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default
    }
};