#pragma once

#include "animation.h"
#include <unordered_set>

struct aiNode;
struct aiBone;
struct aiScene;

struct Joint
{
	std::string name;
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

	inline uint32_t num_bones() { return m_num_joints; }
	inline Joint* joints() { return &m_joints[0]; }

private:
	void build_bone_list(aiNode* node, const aiScene* scene, std::vector<aiBone*>& temp_bone_list, std::unordered_set<std::string>& bone_map);
	void build_skeleton(aiNode* node, int bone_index, const aiScene* scene, std::vector<aiBone*>& temp_bone_list);

private:
	uint32_t		   m_num_joints;
	std::vector<Joint> m_joints;
};