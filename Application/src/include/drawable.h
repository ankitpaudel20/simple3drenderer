#pragma once

#include <utility>

#include "core.h"
#include "Vertexarray.h"
#include "buffer.h"
#include "Shader.h"
#include "material.h"

uint32_t noDrawCalls=0;

inline vec3 operator*(glm::mat4 mat, vec3 vec)
{
	glm::vec4 temp(vec.x, vec.y, vec.z, 1);
	temp = mat * temp;
	return vec3(temp.x, temp.y, temp.z);
}



template <class T>
struct drawable
{
	std::vector<T> m_vertices;
	std::vector<uint32_t> m_indices;
	uint32_t m_primitve = GL_TRIANGLES;
	Material material;
    std::string shader;


	glm::mat4 scaling = glm::mat4(1.0);
	glm::mat4 translation = glm::mat4(1.0);
	glm::mat4 rotation = glm::mat4(1.0);
	glm::mat4 matModel = glm::mat4(1.0);
	
	drawable() = default;

	drawable(const std::vector<T> &vertices, std::vector<unsigned> indices, std::vector<std::string> tex) : m_vertices(vertices), m_indices(std::move(indices))
	{
	    material.diffuseMap=tex[0];
        material.specularMap=tex[1];
        material.normalMap=tex[2];
        material.heightMap=tex[3];
	}

	drawable(const std::vector<vec3> &positions, const std::vector<Indexdata> &indices,std::string shaderPath):shader(std::move(shaderPath))
	{

		uint totalindexcount = 0;
		uint totalvertexcount = 0;
		for (auto &index : indices)
		{
			if (index.primitive.size() < 3)
			{
				printf("index data not valid");
				assert(false);
			}
			totalindexcount += (index.primitive.size() - 2) * 3;
			for (size_t j = 0; j < index.primitive.size(); j++)
				totalvertexcount++;
		}

		m_vertices.reserve(totalvertexcount);
		m_indices.reserve(totalindexcount);

		for (auto &index : indices)
		{
			if (!index.texCoord.empty())
			{
				if (index.texCoord.size() != index.primitive.size())
				{
					std::cout << "texture coord not defined for all indices\n";
					assert(false);
				}
				if (index.tex_id == 0)
					std::cout << "texture id set to zero so texture not used";
			}
			else if (index.tex_id == 0)
			{
				std::cout << "the drawable doesnot have textures\n";
				//index.texCoord = std::vector<vec2>(size, 0);
			}

			for (size_t j = 0; j < index.primitive.size(); j++)
			{
				int pos_index = index.primitive[j];
				T v1(positions[pos_index],  vec3(0));
				m_vertices.push_back(v1);
			}
		}

		unsigned count = 0;

		for (auto &index : indices)
		{
			int size = index.primitive.size();
			int temp = 0;
			for (uint32_t j = count; j < count + size - 2; j++)
			{
				m_indices.push_back(count);
				m_indices.push_back(j + 1);
				m_indices.push_back(j + 2);
				temp++;
				vec3 p1 = m_vertices[count].position;
				vec3 p2 = m_vertices[j + 1].position;
				vec3 p3 = m_vertices[j + 2].position;
				vec3 u = p2 - p1;
				vec3 v = p3 - p2;
				vec3 w = p1 - p3;
				m_vertices[count].normal = vec3::cross(v, w);
				m_vertices[j + 1].normal = vec3::cross(w, u);
				m_vertices[j + 2].normal = vec3::cross(u, v);
			}
			count = count + temp + 2;
		}
	}

	glm::mat4* refreshModel(){
        matModel = translation * rotation * scaling;
        return &matModel;
    }

	// drawable(drawable &&in) noexcept : m_vertices(std::move(in.m_vertices)),m_indices(std::move(in.m_indices)), m_tex(std::move(in.m_tex))
	// {
	// 	m_primitve = in.m_primitve;
	// 	shader=in.shader;
	// }
	// drawable(const drawable &in) = delete;

//	~drawable()
//	{
//		for (auto &a : m_tex)
//			a.free();
//	}
};