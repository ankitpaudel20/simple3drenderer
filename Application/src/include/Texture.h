#pragma once

#include "stb_image/stb_image.h"
#include <algorithm>
#include <string>
#include <utility>

#include "core.h"

std::vector<uint32_t> activeTextures;

class Texture {
private:
  std::string m_filepath;

  int m_width, m_height, m_BPP;

  void LoadFromFile() {
    unsigned char *m_LocalBuffer;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(1);

    m_LocalBuffer = stbi_load(m_filepath.c_str(), &m_width, &m_height, &m_BPP, 4);
    if (!m_LocalBuffer) {
      std::cout << "texture file unable to load" << std::endl;
      assert(false);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer);
    glGenerateMipmap(GL_TEXTURE_2D);
    Texture::Unbind();
    stbi_image_free(m_LocalBuffer);
  }

  void createTexture() {
    glGenTextures(1, &ID);
    activeTextures.push_back(ID);
  }

  void deleteTexture() {
    if (ID) {
      glDeleteTextures(1, &ID);
      auto place = std::find(activeTextures.begin(), activeTextures.end(), ID);

      if (place != activeTextures.end())
        activeTextures.erase(place);
      else {
        printf("there was error in recording generated Textures/n");
        DEBUG_BREAK;
      }
      ID = 0;
    }
  }

public:
  unsigned ID = 0;
  std::string type;

  std::string getPath() { return m_filepath; }

  Texture() = default;

  Texture(std::string path) : m_filepath(std::move(path)) {
    createTexture();
    glBindTexture(GL_TEXTURE_2D, ID);
    LoadFromFile();
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  Texture(const Texture &in) = default;

  Texture &operator=(const Texture &in) {
    if (ID) {
      printf("ERROR::already active texture tried to replace.\n");
      DEBUG_BREAK;
    }
    deleteTexture();
    ID = in.ID;
    m_filepath = in.m_filepath;
    m_width = in.m_width;
    m_height = in.m_height;
    m_BPP = in.m_BPP;
    return *this;
  }

  Texture(Texture &&in) noexcept : m_width(in.m_width), m_height(in.m_height), m_BPP(in.m_BPP), type(in.type) {
    if (ID) {
      printf("ERROR::already active texture tried to replace.\n");
      DEBUG_BREAK;
      deleteTexture();
    }
    ID = in.ID;
    m_filepath = std::move(in.m_filepath);
    in.ID = 0;
  }

  void free() {
    deleteTexture();
  }

  static void freeAll() {
    int i = 0;
    while (i < activeTextures.size())
      glDeleteBuffers(1, &activeTextures[i++]);
    if (i)
      printf("deleted %d number of stray VAOs!!\n", i);
  }

  void Bind(unsigned slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, ID);
  }

  static void Unbind(unsigned slot = 0) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
};
