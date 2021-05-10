#include <string>
#include "stb_image/stb_image.h"

#include "core.h"
#include<buffer.h>
#include<Vertexarray.h>
#include"Shader.h"

class cubeMap
{
	std::string m_filepath;
	int m_width, m_height, m_BPP;
public:
	unsigned ID;


	uint32_t getID() const { return ID; }

	std::string getPath() { return m_filepath; }


	cubeMap(const std::string & filepath) :m_filepath(filepath) {
		GLcall(glGenTextures(1, &ID));
		GLcall(glBindTexture(GL_TEXTURE_CUBE_MAP, ID));

		unsigned char* m_LocalBuffer;

		GLcall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
		GLcall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLcall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GLcall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
		GLcall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		std::vector<std::string> faces{
		pathDelemeter + "right.jpg",
		pathDelemeter + "left.jpg",
		pathDelemeter + "top.jpg",
		pathDelemeter + "bottom.jpg",
		pathDelemeter + "front.jpg",
		pathDelemeter + "back.jpg"
		};

		stbi_set_flip_vertically_on_load(0);
		for (unsigned int i = 0; i < faces.size(); i++) 
		{
			auto fullpath = m_filepath + faces[i];
			m_LocalBuffer = stbi_load(fullpath.c_str() , &m_width, &m_height, &m_BPP, 0);

			if (!m_LocalBuffer) {
				std::cout << "skybox texture file unable to load" << std::endl;
				assert(false);
			}

			GLcall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_LocalBuffer));
			stbi_image_free(m_LocalBuffer);
		}

		GLcall(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));
		Unbind();
		GLcall(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));

	}

	static void Unbind(unsigned slot = 0) {
		GLcall(glActiveTexture(GL_TEXTURE0 + slot));
		GLcall(glBindTexture(GL_TEXTURE_2D, 0));
	}

	void bind() {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
	}

	~cubeMap() {
		if (ID)		
			std::cout << "Texture freed\n";
		GLcall(glDeleteTextures(1, &ID));
		ID = 0;
	}
};


class skyBox {
	cubeMap map;
	uint32_t m_vao, m_vbo;
	Shader m_shader;

	std::vector<float> vertices = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};


public:
	skyBox(const std::string & filepath,const std::string shaderPath) :map(filepath),m_shader(shaderPath+pathDelemeter+"skybox.vert", shaderPath + pathDelemeter + "skybox.frag") {
		glGenBuffers(1, &m_vbo);
		glGenVertexArrays(1, &m_vao);

		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void draw(const glm::mat3& view, glm::mat4& projection ) {

		glm::mat4 mView(view);		// remove translation from the view matrix

		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		map.bind();

		m_shader.Bind();
		m_shader.SetUniform<glm::mat4*>("view", &mView);
		m_shader.SetUniform<glm::mat4*>("proj", &projection);
		m_shader.SetUniform<int>("skybox", 0);

		// skybox cube
		glBindVertexArray(m_vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default
	}
};