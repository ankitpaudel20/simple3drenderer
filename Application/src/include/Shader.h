#pragma once
#include "core.h"
#include <string>
#include <unordered_map>

class Shader {
  private:
    std::string m_vertex;
    std::string m_fragment;
    std::string m_geometry;
    std::string m_path;
    std::string m_vertpath;
    std::string m_fragpath;
    std::string m_geopath;
    std::unordered_map<std::string, int> m_UniformLocationCache;

    static void getstring(const std::string &path, std::string &buffer) {
        FILE *filePointer;
        char c;

#ifdef _MSC_VER
        fopen_s(&filePointer, path.c_str(), "r");
#else
        filePointer = fopen(path.c_str(), "r");
#endif
        if (filePointer == NULL) {
            printf("shader file failed to open: %s\n", path.c_str());
            DEBUG_BREAK;
            assert(false);
        } else {
            while ((c = fgetc(filePointer)) != EOF)
                buffer.push_back(c);

            fclose(filePointer);
        }
    }

    unsigned CreateShader() {
        unsigned int programID = glCreateProgram();
        unsigned vs = CompileShader(GL_VERTEX_SHADER, m_vertex);
        unsigned fs = CompileShader(GL_FRAGMENT_SHADER, m_fragment);
        uint32_t gs;
        if (!m_geometry.empty())
            gs = CompileShader(GL_GEOMETRY_SHADER, m_geometry);

        glAttachShader(programID, vs);
        glAttachShader(programID, fs);
        if (!m_geometry.empty())
            glAttachShader(programID, gs);
        glLinkProgram(programID);

        int link_status;
        glGetProgramiv(programID, GL_LINK_STATUS, &link_status);
        if (link_status == GL_FALSE) {
            int length;
            int retlength;
            glGetShaderiv(programID, GL_INFO_LOG_LENGTH, &length);
            //char* message = (char*)_malloca(length * sizeof(char));
            std::string msg;
            glGetProgramInfoLog(programID, length, &retlength, &msg[0]);
            std::cout << "Failed to link program" << std::endl;
            //std::cout << message << std::endl;
            std::cout << msg << std::endl;
            glDeleteProgram(programID);
            DEBUG_BREAK;
            assert(false);
        }

        glValidateProgram(programID);

        glDeleteShader(vs);
        glDeleteShader(fs);
        return programID;
    }

    static unsigned CompileShader(unsigned int type, const std::string &source) {
        unsigned id = glCreateShader(type);
        const char *src = source.c_str();
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);

        int result;
        glGetShaderiv(id, GL_COMPILE_STATUS, &result);

        if (result == GL_FALSE) {
            int length;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
#ifdef _MSC_VER
            char *message = (char *)_malloca(length * sizeof(char));
#else
            char *message = (char *)malloc(length * sizeof(char));
#endif
            glGetShaderInfoLog(id, length, &length, message);
            std::cout << "falied to comiple" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
            std::cout << message << std::endl;
            glDeleteShader(id);
            DEBUG_BREAK;
            assert(false);
        }

        return id;
    }

    void setshader(const std::string &path) {
        enum class status {
            vertex,
            fragment,
            idle
        };

        FILE *filePointer;

        char c;
        char previous_c = '\0';

        int w_count = 0;
        int c_count = 0;
        int w_size = 0;

        int temp_space = 0;

        status s = status::idle;

#ifdef _MSC_VER
        fopen_s(&filePointer, path.c_str(), "r");
#else
        filePointer = fopen(path.c_str(), "r");
#endif

        std::string test1 = "#shader vertex\n";
        std::string test2 = "#shader fragment\n";

        std::string temp;

        temp.reserve(sizeof(char) * test2.size());

        if (filePointer == nullptr) {
            printf("shader file failed to open\n");
            DEBUG_BREAK;
            assert(false);
        } else {
            while ((c = fgetc(filePointer)) != EOF) {

                c_count++;

                if (w_size <= temp.size())
                    temp.push_back(c);

                w_size++;

                if (s == status::vertex) {
                    m_vertex.push_back(c);
                } else if (s == status::fragment) {
                    m_fragment.push_back(c);
                }

                if (c == ' ' || c == '\n') {

                    if (previous_c != ' ' && previous_c != '\n')
                        temp_space++;
                    else {
                        if (s == status::vertex) {
                            m_vertex.pop_back();
                            if (!temp.empty())
                                temp.pop_back();
                        } else if (s == status::fragment) {
                            m_fragment.pop_back();
                            if (!temp.empty())
                                temp.pop_back();
                        }
                    }
                }

                if (temp_space == 2 || c == '\n') {

                    if (temp == test2) {
                        m_vertex.erase(m_vertex.find(test2));
                        s = status::fragment;
                    } else if (temp == test1) {
                        s = status::vertex;
                    }
                    temp.clear();

                    w_size = 0;
                    w_count++;
                    temp_space = 0;
                }

                previous_c = c;
            }

            fclose(filePointer);
        }
    }

    void setshader2(const std::string &vert, const std::string &frag, const std::string &geo = std::string()) {
        m_vertpath = vert;
        m_fragpath = frag;
        getstring(vert, m_vertex);
        getstring(frag, m_fragment);
        if (!geo.empty()) {
            m_geopath = geo;
            getstring(geo, m_geometry);
        }
    }

    void prepare() {
        if (m_vertex.back() != '\0')
            m_vertex.push_back('\0');
        if (m_fragment.back() != '\0')
            m_fragment.push_back('\0');
        if (m_path.back() != '\0')
            m_path.push_back('\0');
        if (m_vertpath.back() != '\0')
            m_vertpath.push_back('\0');
        if (m_fragpath.back() != '\0')
            m_fragpath.push_back('\0');

        if (!m_geopath.empty()) {
            if (m_geopath.back() != '\0')
                m_geopath.push_back('\0');
            if (m_geometry.back() != '\0')
                m_geometry.push_back('\0');
        }

        m_vertex.shrink_to_fit();
        m_fragment.shrink_to_fit();
        m_geometry.shrink_to_fit();
        m_path.shrink_to_fit();
        m_vertpath.shrink_to_fit();
        m_fragpath.shrink_to_fit();
        m_geopath.shrink_to_fit();
    }

  public:
    std::string name;
    unsigned id = 0;

    int GetUniformLocation(const char *name) {
        if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
            return m_UniformLocationCache[name];

        int location = glGetUniformLocation(id, name);
        if (location == -1)
            std::cout << "Warning: uniform " << name << " does not exist !" << std::endl;

        m_UniformLocationCache[name] = location;
        return location;
    }

    void SetUniform1i(const char *name, int value) {
        (glUniform1i(GetUniformLocation(name), value));
    }

    Shader() {
        printf("default constructor used to make shader \n");
    };

    explicit Shader(const std::string &name, const std::string &path, bool different = false) : id(0), m_path(path), name(name) {
        if (!different) {
            setshader(path);
        } else {
            bool geoexists = false;
            if (FILE *file = fopen((path + ".geom").c_str(), "r")) {
                fclose(file);
                geoexists = true;
            }
            setshader2(path + ".vert", path + ".frag", geoexists ? path + ".geom" : std::string());
        }
        id = CreateShader();
        prepare();
    }

    Shader(const std::string &name, const std::string &vert, const std::string &frag, const std::string &geo = std::string()) : m_path(vert), id(0), name(name) {
        setshader2(vert, frag, geo);
        id = CreateShader();
        prepare();
    }

    Shader(Shader &&in) noexcept {
        id = in.id;
        in.id = 0;
        m_UniformLocationCache = std::move(in.m_UniformLocationCache);
        m_vertex = std::move(in.m_vertex);
        m_fragment = std::move(in.m_fragment);
        m_path = std::move(in.m_path);
        m_vertpath = std::move(in.m_vertpath);
        m_fragpath = std::move(in.m_fragpath);
        name = std::move(in.name);
    }

    Shader &operator=(Shader &&in) noexcept {
        id = in.id;
        in.id = 0;
        m_UniformLocationCache = std::move(in.m_UniformLocationCache);
        m_vertex = std::move(in.m_vertex);
        m_fragment = std::move(in.m_fragment);
        m_path = std::move(in.m_path);
        m_vertpath = std::move(in.m_vertpath);
        m_fragpath = std::move(in.m_fragpath);
        name = std::move(in.name);
        return *this;
    }

    /*Shader &operator=(const Shader &in) noexcept {
        id = in.id;        
        const_cast<Shader&>(in).id = 0;
        m_UniformLocationCache = std::move(in.m_UniformLocationCache);
        m_vertex = std::move(in.m_vertex);
        m_fragment = std::move(in.m_fragment);
        m_path = std::move(in.m_path);
        m_vertpath = std::move(in.m_vertpath);
        m_fragpath = std::move(in.m_fragpath);
        return *this;
    }

    Shader(const Shader &in) noexcept {
        id = in.id;
        const_cast<Shader &>(in).id = 0;
        m_UniformLocationCache = std::move(in.m_UniformLocationCache);
        m_vertex = std::move(in.m_vertex);
        m_fragment = std::move(in.m_fragment);
        m_path = std::move(in.m_path);
        m_vertpath = std::move(in.m_vertpath);
        m_fragpath = std::move(in.m_fragpath);
    }*/

    void free() {
        // std::cout << "shader deleted\n";
        (glUseProgram(0));
        (glDeleteProgram(id));
        id = 0;
    }

    void Bind() const { (glUseProgram(id)); }

    static void Unbind() { (glUseProgram(0)); }

    template <class T>
    void SetUniform(const char *name, T value, unsigned count = 1);

#ifdef _MSC_VER
    template <>
    void SetUniform<int *>(const char *name, int *value, unsigned count);

    template <>
    void SetUniform<int>(const char *name, int value, unsigned count);

    template <>
    void SetUniform<float *>(const char *name, float *value, unsigned count);

    template <>
    void SetUniform<glm::mat4 *>(const char *name, glm::mat4 *value, unsigned count);

    template <>
    void SetUniform<vec3>(const char *name, vec3 value, unsigned count);

    /*template<>
	void SetUniform<mat4*>(const char* name, mat4* value, unsigned count);*/

    template <>
    void SetUniform<float>(const char *name, float value, unsigned count);

    template <>
    void SetUniform<uint32_t>(const char *name, uint32_t value, unsigned count);

    template <>
    void SetUniform<uint32_t *>(const char *name, uint32_t *value, unsigned count);

#endif

    void SetUniform4f(const char *name, float *value) {
        glUniform1fv(GetUniformLocation(name), 4, value);
    }
};

template <class T>
inline void Shader::SetUniform(const char *name, T value, unsigned count) {
    ASSERT(false);
}

template <>
inline void Shader::SetUniform<float *>(const char *name, float *value, unsigned count) {
    glUniform1fv(GetUniformLocation(name), count, value);
}

template <>
inline void Shader::SetUniform<float>(const char *name, float value, unsigned count) {
    glUniform1f(GetUniformLocation(name), value);
}

template <>
inline void Shader::SetUniform<int *>(const char *name, int *value, unsigned count) {
    glUniform1iv(GetUniformLocation(name), count, value);
}

template <>
inline void Shader::SetUniform<int>(const char *name, int value, unsigned count) {
    glUniform1i(GetUniformLocation(name), value);
}

template <>
inline void Shader::SetUniform<uint32_t>(const char *name, uint32_t value, unsigned count) {
    glUniform1ui(GetUniformLocation(name), value);
}

template <>
inline void Shader::SetUniform<uint32_t *>(const char *name, uint32_t *value, unsigned count) {
    glUniform1uiv(GetUniformLocation(name), count, value);
}

template <>
inline void Shader::SetUniform<const glm::mat4 &>(const char *name, const glm::mat4 &value, unsigned count) {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &(value)[0][0]);
}

//template<>
//inline void Shader::SetUniform<mat4*>(const char* name, mat4* value, unsigned count) {
//	(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &(*value)[0][0]));
//}

template <>
inline void Shader::SetUniform<vec3>(const char *name, vec3 value, unsigned count) {
    glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
}
