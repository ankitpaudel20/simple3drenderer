#pragma once

#include <vector>
#include <cstring>
#include "core.h"

std::vector<uint32_t> activeVBO;
std::vector<uint32_t> activeIBO;

template<class T, uint32_t TYPE>
struct buffer {
private:
    std::vector<uint32_t> *list;

    void createBuffer() {

        glGenBuffers(1, &id);
        list->push_back(id);
    }

    void deleteBuffer() {
        if (id) {
            glDeleteVertexArrays(1, &id);
            auto place = std::find(list->begin(), list->end(), id);

            if (place != list->end())
                list->erase(place);
            else {
                printf("there was error in recording generated %ss/n", TYPE == GL_ARRAY_BUFFER ? "vbo" : "ibo");
                DEBUG_BREAK;
            }

            id = 0;
        }
    }

public:

    uint32_t id = 0;
    uint32_t m_count;


    void setup(const std::vector<T> &data) {
        m_count = data.size();

        deleteBuffer();
        createBuffer();

        GLcall(glBindBuffer(TYPE, id));
        GLcall(glBufferData(TYPE, sizeof(T) * data.size(), &data[0], GL_STATIC_DRAW));
        GLcall(glBindBuffer(TYPE, 0));
    }

    buffer() : id(0), m_count(0) {
        if (TYPE == GL_ARRAY_BUFFER)
            list = &activeVBO;
        else
            list = &activeIBO;
    }

    explicit buffer(const std::vector<T> &data) {
        if (TYPE == GL_ARRAY_BUFFER)
            list = &activeVBO;
        else
            list = &activeIBO;

        if (data.empty()) {
            std::cout << "buffer tried to initialize with empty vertex data\n";
            id = 0;
            m_count = 0;
            return;
        }
        setup(data);
    }

#pragma region moveCopyConstructors
    buffer(const buffer &in) {
        if (id) {
            printf("ERROR::already active %s buffer tried to replace.", TYPE == GL_ARRAY_BUFFER ? "vertex" : "index");
            DEBUG_BREAK;
        }
        id = in.id;
        m_count = in.m_count;
        list = in.list;
    }

    buffer& operator =(const buffer &in){
        if (id) {
            printf("ERROR::already active %s buffer tried to replace.", TYPE == GL_ARRAY_BUFFER ? "vertex" : "index");
            DEBUG_BREAK;
        }
        id = in.id();
        m_count = in.m_count;
        list = in.list;
    }

    buffer& operator =(buffer &&in){
        if (id) {
            printf("ERROR::already active %s buffer tried to replace.\n", TYPE == GL_ARRAY_BUFFER ? "vertex" : "index");
            DEBUG_BREAK;
        }
        id = in.id;
        in.id=0;
        m_count = in.m_count;
        list = in.list;
        return *this;
    }

    buffer(buffer &&in)  noexcept {
        id = in.id();
        in.id=0;
        m_count = in.m_count;
        list = in.list;
    }

#pragma endregion

    void free() {
        deleteBuffer();
        id = 0;
    }

    void Bind() const {
        GLcall(glBindBuffer(TYPE, id));
    }

    void Unbind() const {
        GLcall(glBindBuffer(TYPE, 0));
    }

    void AddData(const std::vector<T> &data) {
        if (data.empty()) {
            std::cout << "data tried to be added into buffer with empty data\n";
            assert(false);
            return;
        }

        if (id == 0) {
            setup(data);
        } else {


            m_count = m_count + data.size();

            GLcall(glBindBuffer(TYPE, id));
            GLcall(void *pointer = glMapBuffer(TYPE, GL_READ_ONLY));
            int buffersize;
            GLcall(glGetBufferParameteriv(TYPE, GL_BUFFER_SIZE, &buffersize));

#ifdef _MSC_VER
            void *tempdata = _malloca(m_count * sizeof(T));
            memcpy_s(tempdata, m_count * sizeof(T), pointer, m_count * sizeof(T));
#else
            void *tempdata = malloc(buffersize);
            memcpy(tempdata, pointer, buffersize);
#endif
            deleteBuffer();
            createBuffer();
            GLcall(glBindBuffer(TYPE, id));
            GLcall(glBufferData(TYPE, buffersize + data.size() * sizeof(T), nullptr, GL_STATIC_DRAW));
            GLcall(glBufferSubData(TYPE, 0, buffersize, tempdata));
            GLcall(glBufferSubData(TYPE, buffersize, sizeof(T) * data.size(), &data[0]));
            GLcall(glBindBuffer(TYPE, 0));
        }
    }

    [[nodiscard]] inline uint32_t get_count() const { return m_count; }

    [[nodiscard]] bool isempty() const { return m_count == 0; }

    static void freeAll() {
        int i = 0;
        std::vector<uint32_t> *list;

        if (TYPE == GL_ARRAY_BUFFER)
            list = &activeVBO;
        else
            list = &activeIBO;

        while (i < list->size())
            glDeleteBuffers(1, &(*list)[i++]);
        if (i)
            printf("deleted %d number of stray %ss!!\n", i, TYPE == GL_ARRAY_BUFFER ? "vbo" : "ibo");

    }
};
