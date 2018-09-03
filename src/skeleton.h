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

private:
	Skeleton();
	~Skeleton();
	void build_bone_list(aiNode* node, int bone_index, const aiScene* scene, std::vector<aiBone*>& temp_bone_list);
	void build_skeleton(aiNode* node, int bone_index, const aiScene* scene);
	int32_t find_joint_index(const std::string& channel_name);

private:
	uint16_t		   num_joints;
	uint32_t		   num_raw_bones;
	std::vector<Joint> joints;
};