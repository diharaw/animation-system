#include "anim_lookat_ik.h"
#include <algorithm>
#include <gtc/matrix_transform.hpp>
#include <iostream>
#include <imgui.h>

AnimLookAtIK::AnimLookAtIK(Skeleton* skeleton) : m_skeleton(skeleton)
{

}

AnimLookAtIK::~AnimLookAtIK()
{

}

PoseTransforms* AnimLookAtIK::look_at(PoseTransforms* input, PoseTransforms* input_local, const glm::vec3& target, float max_angle, const std::string& joint)
{
	int32_t idx = m_skeleton->find_joint_index(joint);

	if (idx != -1)
	{
		glm::mat4 bone_mat = input->transforms[idx];
		bone_mat[3][0] = 0.0f;
		bone_mat[3][1] = 0.0f;
		bone_mat[3][2] = 0.0f;
		bone_mat[3][3] = 1.0f;

		glm::mat4 to_bone_space = glm::inverse(bone_mat);

		glm::vec4 bone_fwd = to_bone_space * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		glm::vec3 bone_fwd_dir = glm::normalize(glm::vec3(bone_fwd.x, bone_fwd.y, bone_fwd.z));

		glm::mat4 global_to_local = glm::inverse(input->transforms[idx]);

		glm::vec4 local_target = to_bone_space * glm::vec4(target, 1.0f);
		glm::vec3 local_target_dir = glm::normalize(glm::vec3(local_target.x, local_target.y, local_target.z));

		glm::vec3 rotation_axis = glm::cross(bone_fwd_dir, local_target_dir);
		rotation_axis = glm::normalize(rotation_axis);

		float angle = acosf(glm::dot(local_target_dir, bone_fwd_dir));

		ImGui::Text("Angle: %f, Target: [%f, %f, %f], Bone: [%f, %f, %f]", glm::degrees(angle), local_target_dir.x, local_target_dir.y, local_target_dir.z, bone_fwd_dir.x, bone_fwd_dir.y, bone_fwd_dir.z);
		//glm::vec3 diff = bone_fwd_dir - local_target_dir;
		//ImGui::Text("Axis: [%f, %f, %f]", rotation_axis.x, rotation_axis.y, rotation_axis.z);

		angle = std::min(angle, glm::radians(max_angle));

		glm::mat4 rotation_mat = glm::rotate(glm::mat4(1.0f), angle, rotation_axis);
		input->transforms[idx] = input->transforms[idx] * rotation_mat;

		Joint* joints = m_skeleton->joints();

		for (uint32_t i = (idx + 1); i < m_skeleton->num_bones(); i++)
		{
			if (i > idx && joints[i].parent_index < idx)
				break;

			if (joints[i].parent_index >= idx)
			{
				if (joints[i].parent_index == -1)
					input->transforms[i] = input_local->transforms[i];
				else
					input->transforms[i] = input->transforms[joints[i].parent_index] * input_local->transforms[i];
			}
		}

		return input;
	}

	return nullptr;
}