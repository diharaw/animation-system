#include "anim_fabrik_ik.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>
#include <logger.h>

// https://zalo.github.io/blog/inverse-kinematics/
glm::quat rotation_from_two_vectors(glm::vec3 u, glm::vec3 v)
{
	float norm_u_norm_v = sqrt(glm::dot(u, u) * glm::dot(v, v));
	float real_part = norm_u_norm_v + dot(u, v);
	glm::vec3 w;

	if (real_part < 1.e-6f * norm_u_norm_v)
	{
		/* If u and v are exactly opposite, rotate 180 degrees
		 * around an arbitrary orthogonal axis. Axis normalisation
		 * can happen later, when we normalise the quaternion. */
		real_part = 0.0f;
		w = abs(u.x) > abs(u.z) ? glm::vec3(-u.y, u.x, 0.f) : glm::vec3(0.f, -u.z, u.y);
	}
	else
	{
		/* Otherwise, build quaternion the standard way. */
		w = glm::cross(u, v);
	}

	return glm::normalize(glm::quat(real_part, w.x, w.y, w.z));
}

AnimFabrikIK::AnimFabrikIK(Skeleton* skeleton) : m_skeleton(skeleton)
{

}

AnimFabrikIK::~AnimFabrikIK()
{

}

PoseTransforms* AnimFabrikIK::solve(glm::mat4 model, PoseTransforms* local_transforms, PoseTransforms* global_transforms, const glm::vec3& end_effector, const std::string& start_bone, const std::string& end_bone)
{
	for (int i = 0; i < m_skeleton->num_bones(); i++)
		m_transforms.transforms[i] = global_transforms->transforms[i];

	int32_t start_idx = m_skeleton->find_joint_index(start_bone);

	if (start_idx == -1)
	{
		DW_LOG_ERROR("FABRIK IK: Requested start bone not found = " + start_bone);
		return global_transforms;
	}

	int32_t end_idx = m_skeleton->find_joint_index(end_bone);

	if (end_idx == -1)
	{
		DW_LOG_ERROR("FABRIK IK: Requested end bone not found = " + end_bone);
		return global_transforms;
	}

	int32_t local_end_idx = find_source_chain_data(model, start_idx, end_idx, global_transforms);

	for (uint32_t i = 0; i < m_iterations; i++)
	{
		backward_ik(local_end_idx, end_effector);
		forward_ik(local_end_idx, end_effector);
	}

	modify_transforms(model, start_idx, end_idx, local_transforms);

	return &m_transforms;
}

int32_t AnimFabrikIK::find_source_chain_data(glm::mat4 model, int32_t start_idx, int32_t end_idx, PoseTransforms* global_transforms)
{
	int32_t count = 0;

	for (int32_t i = start_idx; i <= end_idx; i++)
	{
		int32_t idx = count++;

		glm::mat4 m = model * global_transforms->transforms[i];

		m_source_joint_pos[idx] = glm::vec3(m[3][0], m[3][1], m[3][2]);
		m_iteration_joint_pos[idx] = m_source_joint_pos[idx];
	}

	for (int32_t i = 0; i < (count - 1); i++)
		m_bone_lengths[i] = glm::length(m_source_joint_pos[i] - m_source_joint_pos[i + 1]);

	return count - 1;
}

void AnimFabrikIK::forward_ik(int32_t end_idx, const glm::vec3& end_effector)
{
	for (int32_t i = 0; i <= end_idx; i++)
	{
		if (i == 0)
			m_iteration_joint_pos[i] = m_source_joint_pos[i];
		else
		{
			glm::vec3 dir = glm::normalize(m_iteration_joint_pos[i] - m_iteration_joint_pos[i - 1]);
			m_iteration_joint_pos[i] = m_iteration_joint_pos[i - 1] + dir * m_bone_lengths[i - 1];
		}
	}
}

void AnimFabrikIK::backward_ik(int32_t end_idx, const glm::vec3& end_effector)
{
	for (int32_t i = end_idx; i >= 0; i--)
	{
		if (i == end_idx)
			m_iteration_joint_pos[i] = end_effector;
		else
		{
			glm::vec3 dir = glm::normalize(m_iteration_joint_pos[i] - m_iteration_joint_pos[i + 1]);
			m_iteration_joint_pos[i] = m_iteration_joint_pos[i + 1] + dir * m_bone_lengths[i];
		}
	}
}

void AnimFabrikIK::modify_transforms(glm::mat4 model, int32_t start_idx, int32_t end_idx, PoseTransforms* local_transforms)
{
	glm::mat4 inv_model = glm::inverse(model);

	for (int32_t i = start_idx; i < end_idx; i++)
	{
		// Calculate the vector pointing from the one joint to the next in the source transforms.
		glm::vec3 src_joint_start_pos = glm::vec3(m_transforms.transforms[i][3][0], m_transforms.transforms[i][3][1], m_transforms.transforms[i][3][2]);
		glm::vec3 src_joint_end_pos = glm::vec3(m_transforms.transforms[i + 1][3][0], m_transforms.transforms[i + 1][3][1], m_transforms.transforms[i + 1][3][2]);
		glm::vec3 src_joint_dir = glm::normalize(glm::vec3(src_joint_end_pos) - glm::vec3(src_joint_start_pos));

		// Calculate the vector pointing from the one joint to the next in the IK transforms.
		glm::vec4 dst_joint_start_pos = inv_model * glm::vec4(m_iteration_joint_pos[i - start_idx], 1.0f);
		glm::vec4 dst_joint_end_pos = inv_model * glm::vec4(m_iteration_joint_pos[i - start_idx + 1], 1.0f);
		glm::vec3 dst_joint_dir = glm::normalize(glm::vec3(dst_joint_end_pos) - glm::vec3(dst_joint_start_pos));

		// Find the quaternion that rotates from source to destination rotations.
		glm::quat src_to_dst_rotation = rotation_from_two_vectors(src_joint_dir, dst_joint_dir);

		// Find the original rotation of the joint.
		glm::quat origin_rotation = glm::normalize(glm::quat_cast(m_transforms.transforms[i]));

		// Create a translation matrix from the destination joint position.
		glm::mat4 translation = glm::mat4(1.0f);
		translation = glm::translate(translation, glm::vec3(dst_joint_start_pos.x, dst_joint_start_pos.y, dst_joint_start_pos.z));

		// Compute the final joint rotation by adding to the original rotation.
		glm::quat final_rotation = glm::normalize(src_to_dst_rotation * origin_rotation);
		glm::mat4 rotation = glm::mat4_cast(final_rotation);

		// Compute final joint transform.
		m_transforms.transforms[i] = translation * rotation;
	}

	Joint* joints = m_skeleton->joints();

	int32_t i = end_idx;

	while (joints[i].parent_index > start_idx)
	{
		if (joints[i].parent_index == -1)
			m_transforms.transforms[i] = local_transforms->transforms[i];
		else
			m_transforms.transforms[i] = m_transforms.transforms[joints[i].parent_index] * local_transforms->transforms[i];

		i++;
	}
}