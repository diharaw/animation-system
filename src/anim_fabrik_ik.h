#pragma once

#include "skeletal_mesh.h"

#define MAX_IK_CHAIN_SIZE 8

class AnimFabrikIK
{
public:
	AnimFabrikIK(Skeleton* skeleton);
	~AnimFabrikIK();

	PoseTransforms* solve(glm::mat4 model, PoseTransforms* local_transforms, PoseTransforms* global_transforms, const glm::vec3& end_effector, const std::string& start_bone, const std::string& end_bone);
	inline uint32_t num_iterations() { return m_iterations; }
	inline void set_iterations(uint32_t itr) { m_iterations = itr; }

private:
	int32_t find_source_chain_data(glm::mat4 model, int32_t start_idx, int32_t end_idx, PoseTransforms* global_transforms);
	void forward_ik(int32_t end_idx, const glm::vec3& end_effector);
	void backward_ik(int32_t end_idx, const glm::vec3& end_effector);
	void modify_transforms(glm::mat4 model, int32_t start_idx, int32_t end_idx, PoseTransforms* local_transforms);

private:
	float m_bone_lengths[MAX_IK_CHAIN_SIZE - 1];
	glm::vec3 m_source_joint_pos[MAX_IK_CHAIN_SIZE];
	glm::vec3 m_iteration_joint_pos[MAX_IK_CHAIN_SIZE];

	uint32_t m_iterations = 16;
	Skeleton* m_skeleton;
	PoseTransforms m_transforms;
};