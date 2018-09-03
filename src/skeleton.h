#pragma once

#include "animation.h"

struct aiNode;
struct aiBone;
struct aiScene;

struct Joint
{
	std::string name;
	glm::mat4   global_transform;
	glm::mat4	offset_transform;
	int32_t		parent_index;
};

class Skeleton
{
public:
	static Skeleton* create(const aiScene* scene);

	Skeleton();
	~Skeleton();
	int32_t find_joint_index(const std::string& channel_name);

	inline uint32_t num_raw_bones() { return m_num_raw_bones; }

private:
	void build_bone_list(aiNode* node, int bone_index, const aiScene* scene, std::vector<aiBone*>& temp_bone_list);
	void build_skeleton(aiNode* node, int bone_index, const aiScene* scene, std::vector<aiBone*>& temp_bone_list);

private:
	uint16_t		   m_num_joints;
	uint32_t		   m_num_raw_bones;
	std::vector<Joint> m_joints;
};