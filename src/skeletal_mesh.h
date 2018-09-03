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

	inline uint32_t raw_bone_count() { return m_raw_bone_count; }
	inline Skeleton* skeleton() { return m_skeletal; }

private:
	dw::Buffer*		 m_ibo;
	dw::Buffer*		 m_vbo;
	dw::VertexArray* m_vao;
	uint32_t		 m_raw_bone_count;
	Skeleton*		 m_skeletal;
};