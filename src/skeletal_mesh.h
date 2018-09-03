#pragma once

#include "skeleton.h"
#include <ogl.h>

struct SkeletalVertex
{
	glm::vec3  position;
	glm::vec2  texcoord;
	glm::vec3  normal;
	glm::vec3  tangent;
	glm::ivec4 bone_indices;
	glm::vec4  bone_weights;
};

struct SkeletalColoredVertex
{
	glm::vec3  position;
	glm::vec2  texcoord;
	glm::vec3  normal;
	glm::vec4  color;
	glm::ivec4 bone_indices;
	glm::vec4  bone_weights;
};

struct SubMesh
{
	uint16_t num_indices = 0;
	uint16_t base_vertex = 0;
	uint16_t base_index = 0;
};

class SkeletalMesh
{
public:
	static SkeletalMesh* load(const std::string& name, Skeleton* skeleton = nullptr);

	SkeletalMesh();
	~SkeletalMesh();
	void bind_vao();

	inline Skeleton* skeleton() { return m_skeleton; }

private:
	dw::Buffer*						   m_ibo;
	dw::Buffer*						   m_vbo;
	dw::VertexArray*				   m_vao;
	std::vector<SubMesh>			   m_sub_meshes;
	std::vector<SkeletalVertex>		   m_vertices;
	std::vector<SkeletalColoredVertex> m_color_vertices;
	std::vector<uint32_t>			   m_indices;
	Skeleton*						   m_skeleton;
	bool							   m_has_vertex_colors;
};