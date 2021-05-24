#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include <cassert>
#include <ctime>
#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
const std::string pathDelemeter(std::string("\\") + "\\");
#else
const std::string pathDelemeter("/");
#endif // _WIN32

#ifdef _MSC_VER
#define DEBUG_BREAK __debugbreak()

#ifdef _DEBUG
#define MDEBUG
#endif
#else
#define DEBUG_BREAK __builtin_trap()

#ifndef NDEBUG
#define MDEBUG
#endif
#endif

//#define _DEBUG

#define ASSERT(x)      \
    if (!(x)) {        \
        DEBUG_BREAK;   \
        assert(false); \
    }

inline void
GLClearError() {
    while (glGetError() != GL_NO_ERROR)
        ;
}

inline bool GLLogCall(const char *function, const char *file, int line) {
    while (GLenum errorcode = glGetError()) {
        std::string error;
        switch (errorcode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            error = "STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            error = "STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << "[Opengl Error] ( Error code :" << errorcode << " : " << error << " )" << function << " " << file << " : " << line << std::endl;
        return false;
    }
    return true;
}

typedef unsigned int uint;

struct vec3 {
    float x, y, z;

    vec3() { x = y = z = 0; }
    vec3(float a) : x(a), y(a), z(a) {}
    vec3(float a, float b, float c) : x(a), y(b), z(c) {}
    vec3(const glm::vec3 &in) : x(in.x), y(in.y), z(in.z) {}
    vec3(glm::vec3 &in) : x(in.x), y(in.y), z(in.z) {}

    float getmag() {
        return sqrtf(pow(x, 2) + pow(y, 2) + pow(z, 2));
    }

    operator glm::vec4() {
        return glm::vec4(x, y, z, 1.0);
    }

    operator glm::vec3() {
        return glm::vec3(x, y, z);
    }
    operator glm::vec4() const {
        return glm::vec4(x, y, z, 1.0);
    }

    operator glm::vec3() const {
        return glm::vec3(x, y, z);
    }

    friend vec3 operator+(const vec3 &a, const vec3 &b) {
        return vec3(a.x + b.x, a.y + b.y, a.z + b.z);
    }

    friend vec3 operator+(vec3 in, float a) {
        return vec3(in.x + a, in.y + a, in.z + a);
    }

    friend vec3 operator-(const vec3 &a, const vec3 &b) {
        return vec3(a.x - b.x, a.y - b.y, a.z - b.z);
    }

    friend vec3 operator-(vec3 in, float a) {
        return vec3(in.x - a, in.y - a, in.z - a);
    }

    friend vec3 operator*(const vec3 &a, const vec3 &b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    friend vec3 operator*(vec3 in, float a) {
        return vec3(in.x * a, in.y * a, in.z * a);
    }

    friend vec3 operator/(const vec3 &a, const vec3 &b) {
        return vec3(a.x / b.x, a.y / b.y, a.z / b.z);
    }

    friend vec3 operator/(const vec3 &in, float a) {
        return vec3(in.x / a, in.y / a, in.z / a);
    }

    vec3 operator-() {
        return vec3(-x, -y, -z);
    }

    vec3 operator+=(const vec3 &in) {
        x += in.x;
        y += in.y;
        z += in.z;
        return *this;
    }

    vec3 operator-=(const vec3 &in) {
        x -= in.x;
        y -= in.y;
        z -= in.z;
        return *this;
    }

    vec3 operator*=(const vec3 &in) {
        x *= in.x;
        y *= in.y;
        z *= in.z;
        return *this;
    }

    vec3 operator/=(const vec3 &in) {
        x /= in.x;
        y /= in.y;
        z /= in.z;
        return *this;
    }

    inline bool operator==(const vec3 &in) {
        return x == in.x && y == in.y && z == in.z;
    }

    inline bool operator!=(const vec3 &in) {
        return !operator==(in);
    }

    friend std::ostream &operator<<(std::ostream &out, const vec3 &vec) {
        out << "( " << vec.x << ", " << vec.y << ", " << vec.z << " )\n";
        return out;
    }

    vec3 normalize() {
        float temp = sqrt(x * x + y * y + z * z);
        *this = *this / temp;
        return *this;
    }

    static vec3 normalize(const vec3 &in) {
        float temp = sqrt(in.x * in.x + in.y * in.y + in.z * in.z);
        return vec3(in / temp);
    }

    static float dist(const vec3 &a, const vec3 &b) {
        return sqrtf(pow(b.x - a.x, 2) + pow(b.y - a.y, 2) + pow(b.z - a.z, 2));
    }

    static vec3 cross(const vec3 &a, const vec3 &b) {
        vec3 ret;
        ret.x = a.y * b.z - b.y * a.z;
        ret.y = b.x * a.z - a.x * b.z;
        ret.z = a.x * b.y - b.x * a.y;
        return ret.normalize();
    }
};

struct vec4 {
    float x, y, z, w;

    vec4() {}
    vec4(float a) : x(a), y(a), z(a), w(a) {}
    vec4(float a, float b) : x(a), y(a), z(a), w(b) {}
    vec4(float a, float b, float c) : x(a), y(b), z(c), w(1) {}
    vec4(float a, float b, float c, float d) : x(a), y(b), z(c), w(d) {}
    vec4(const vec3 &a) : x(a.x), y(a.y), z(a.z), w(1.0) {}

    operator glm::vec4() {
        return glm::vec4(x, y, z, w);
    }

    vec4 operator-() {
        return vec4(-x, -y, -z, -w);
    }
};

struct vec2 {
    float x, y;

    vec2() {}
    vec2(float a) : x(a), y(a) {}
    vec2(float a, float b) : x(a), y(b) {}
    vec2(glm::vec2 a) : x(a.x), y(a.y) {}

    float getmag() {
        return sqrtf(pow(x, 2) + pow(y, 2));
    }

    operator glm::vec2() {
        return glm::vec2(x, y);
    }

    operator glm::vec3() {
        return glm::vec3(x, y, 0);
    }
};

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 texCoord;
    vec3 tangent;
    vec3 bitangent;

    Vertex(const vec3 &pos, const vec3 &nor = 0, const vec2 &texcoord = 0, const vec3 &tangent = 0, const vec3 &bitangent = 0, const float texid = 0)
        : position(pos), normal(nor), tangent(tangent), bitangent(bitangent), texCoord(texcoord) {}
    Vertex() {}
};

struct dirLight {
    vec3 direction;
    float intensity;

    vec3 diffusecolor;
    vec3 specularcolor;
    dirLight(const vec3 &dir, const float &intensity, const vec3 &diffcol = 1, const vec3 &speccol = 1) : direction(dir), diffusecolor(diffcol), specularcolor(speccol), intensity(intensity) {}
};
