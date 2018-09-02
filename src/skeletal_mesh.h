#pragma once

#include "animation.h"
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

struct Joint
{
	std::string name;
	glm::mat4   global_transform;
	glm::mat4	offset_transform;
	int32_t		parent_index;
};

struct Skeleton
{
	uint16_t		   num_joints;
	std::vector<Joint> joints;
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
	static SkeletalMesh* load(const std::string& name);

	SkeletalMesh();
	~SkeletalMesh();
	void bind_vao();
	int32_t find_joint_index(const std::string& channel_name);
	inline uint32_t raw_bone_count() { return m_raw_bone_count; }

private:
	dw::Buffer*		 m_ibo;
	dw::Buffer*		 m_vbo;
	dw::VertexArray* m_vao;
	uint32_t		 m_raw_bone_count;
};