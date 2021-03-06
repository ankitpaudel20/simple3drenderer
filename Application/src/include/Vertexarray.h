#pragma once

#include "VertexBufferLayout.h"
#include "buffer.h"
#include <vector>

std::vector<uint32_t> activeVAO;
class Vertexarray;

class Vertexarray {
private:
  static std::vector<uint32_t> *list;
  unsigned m_arrayID = 0;
  unsigned attrib_count{};
  VertexBufferLayout m_layout;

  void m_AddBuffer(const unsigned &vbo_id, const unsigned &ibo_id) {
    if (!m_arrayID) {
      init();
    }
    glBindVertexArray(m_arrayID);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
    const std::vector<VertexBufferElement> &elements = m_layout.GetElements();
    unsigned offset = 0;
    unsigned i = 0;
    while (i < elements.size()) {
      const VertexBufferElement &element = elements[i];
      glEnableVertexAttribArray(i);
      glVertexAttribPointer(i, element.count, element.type, element.normalized, m_layout.GetStride(), (const void *)offset);
      offset += element.count * element.GetTypeSize();
      i++;
    }
    attrib_count = i;
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  static void createarray(unsigned &id) {
    glGenVertexArrays(1, &id);
    list->push_back(id);
  }

  static void deletearray(uint32_t &id) {
    if (id) {
      glDeleteVertexArrays(1, &id);
      auto place = std::find(list->begin(), list->end(), id);

      if (place != list->end())
        list->erase(place);
      else {
        printf("there was error in recording generated vaos/n");
        DEBUG_BREAK;
      }

      id = 0;
    }
  }

public:
  Vertexarray() {
    list = &activeVAO;
  }

  Vertexarray(buffer<Vertex, GL_ARRAY_BUFFER> &vbo, buffer<uint32_t, GL_ELEMENT_ARRAY_BUFFER> &ibo);

  void init() {
    deletearray(m_arrayID);
    createarray(m_arrayID);
  }

  void free() {
    deletearray(m_arrayID);
  }

  template <class T>
  void addBuffer(const buffer<T, GL_ARRAY_BUFFER> &vbo, const buffer<uint32_t, GL_ELEMENT_ARRAY_BUFFER> &ibo) {
    assert(false);
  }

  //    template <>
  //    void addBuffer<Vertex>(const buffer<Vertex, GL_ARRAY_BUFFER> &vbo, const buffer<uint32_t, GL_ELEMENT_ARRAY_BUFFER> &ibo){
  //        m_layout.push<float>(3, GL_FALSE);
  //        m_layout.push<float>(4, GL_FALSE);
  //        m_layout.push<float>(3, GL_FALSE);
  //        m_layout.push<float>(2, GL_FALSE);
  //        m_layout.push<float>(1, GL_FALSE);
  //        m_AddBuffer(vbo.id, ibo.id);
  //    }

#ifdef _MSC_VER
#endif

  void AddBufferf(const buffer<float, GL_ARRAY_BUFFER> &vb, const VertexBufferLayout &layout) {
    Bind();
    vb.Bind();
    const std::vector<VertexBufferElement> &elements = layout.GetElements();
    unsigned offset = 0;
    unsigned i = 0;
    while (i < elements.size()) {

      const VertexBufferElement &element = elements[i];
      glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.GetStride(), (const void *)offset);
      glEnableVertexAttribArray(i);
      offset += element.count * element.GetTypeSize();
      i++;
    }
    attrib_count = i;
    vb.Unbind();
    Unbind();
  }

  void Bind() const {
    glBindVertexArray(m_arrayID);
  }

  static void Unbind() {
    glBindVertexArray(0);
  }

  static void Unbind2(const unsigned &position) {
    glBindVertexArray(0);
    glDisableVertexAttribArray(position);
  }

  [[nodiscard]] unsigned get() const { return m_arrayID; }

  //    Vertexarray(Vertexarray &&in) noexcept: m_layout(in.m_layout) {
  //        m_arrayID = in.m_arrayID;
  //        attrib_count = in.attrib_count;
  //        in.m_arrayID = 0;
  //    }
  //
  //    Vertexarray(Vertexarray &in) {
  //
  //        m_arrayID=in.m_arrayID;
  //        m_layout=in.m_layout;
  //
  //
  //        if (in.m_arrayID!=0){
  //            printf("copied to already created vertexarray\n");
  //            DEBUG_BREAK;
  //        }
  //
  //    }
  static void freeAll() {
    int i = 0;
    while (i < list->size())
      glDeleteBuffers(1, &(*list)[i++]);
    if (i)
      printf("deleted %d number of stray VAOs!!\n", i);
  }
};

template <>
void Vertexarray::addBuffer<Vertex>(const buffer<Vertex, GL_ARRAY_BUFFER> &vbo,
                                    const buffer<uint32_t, GL_ELEMENT_ARRAY_BUFFER> &ibo) {
  m_layout.push<float>(3, GL_FALSE);
  m_layout.push<float>(3, GL_FALSE);
  m_layout.push<float>(2, GL_FALSE);
  m_layout.push<float>(3, GL_FALSE);
  m_layout.push<float>(3, GL_FALSE);
  m_AddBuffer(vbo.id, ibo.id);
}

Vertexarray::Vertexarray(buffer<Vertex, 0x8892> &vbo, buffer<uint32_t, 0x8893> &ibo) {
  this->init();
  this->addBuffer<Vertex>(vbo, ibo);
}

std::vector<uint32_t> *Vertexarray::list = &activeVAO;
