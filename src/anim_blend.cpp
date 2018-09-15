#include "anim_blend.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/compatibility.hpp>

AnimBlend::AnimBlend(Skeleton* skeleton) : m_skeleton(skeleton)
{

}

AnimBlend::~AnimBlend()
{

}

Pose* AnimBlend::blend(Pose* base, Pose* secondary, float t)
{
	m_pose.num_keyframes = base->num_keyframes;

	for (uint32_t i = 0; i < base->num_keyframes; i++)
	{
		m_pose.keyframes[i].translation = glm::lerp(base->keyframes[i].translation, secondary->keyframes[i].translation, t);
		m_pose.keyframes[i].rotation = glm::slerp(base->keyframes[i].rotation, secondary->keyframes[i].rotation, t);
		m_pose.keyframes[i].scale = glm::lerp(base->keyframes[i].scale, secondary->keyframes[i].scale, t);
	}

	return &m_pose;
}

Pose* AnimBlend::blend_partial(Pose* base, Pose* secondary, float t, const std::string& root_joint)
{
	m_pose.num_keyframes = base->num_keyframes;

	uint32_t idx = m_skeleton->find_joint_index(root_joint);
	bool found = false;
	Joint* joints = m_skeleton->joints();

	if (idx >= 0)
	{
		for (uint32_t i = 0; i < base->num_keyframes; i++)
		{
			if (i == idx)
				found = true;

			if (i > idx && joints[i].parent_index < idx)
				found = false;

			if (found && (joints[i].parent_index >= idx || i == idx))
			{
				m_pose.keyframes[i].translation = glm::lerp(base->keyframes[i].translation, secondary->keyframes[i].translation, t);
				m_pose.keyframes[i].rotation = glm::slerp(base->keyframes[i].rotation, secondary->keyframes[i].rotation, t);
				m_pose.keyframes[i].scale = glm::lerp(base->keyframes[i].scale, secondary->keyframes[i].scale, t);
			}
			else
			{
				m_pose.keyframes[i].translation = base->keyframes[i].translation;
				m_pose.keyframes[i].rotation = base->keyframes[i].rotation;
				m_pose.keyframes[i].scale = base->keyframes[i].scale;
			}
		}
	}

	return &m_pose;
}

Pose* AnimBlend::blend_additive(Pose* base, Pose* secondary, float t)
{
	m_pose.num_keyframes = base->num_keyframes;

	for (uint32_t i = 0; i < base->num_keyframes; i++)
	{
		m_pose.keyframes[i].translation = base->keyframes[i].translation + secondary->keyframes[i].translation * t;
		m_pose.keyframes[i].rotation = base->keyframes[i].rotation * glm::slerp(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), secondary->keyframes[i].rotation, t);
		m_pose.keyframes[i].scale = base->keyframes[i].scale + secondary->keyframes[i].scale * t;
	}

	return &m_pose;
}

Pose* AnimBlend::blend_partial_additive(Pose* base, Pose* secondary, float t, const std::string& root_joint)
{
	m_pose.num_keyframes = base->num_keyframes;

	uint32_t idx = m_skeleton->find_joint_index(root_joint);
	bool found = false;
	Joint* joints = m_skeleton->joints();

	if (idx >= 0)
	{
		for (uint32_t i = 0; i < base->num_keyframes; i++)
		{
			if (i == idx)
				found = true;

			if (i > idx && joints[i].parent_index < idx)
				found = false;

			if (found && (joints[i].parent_index >= idx || i == idx))
			{
				m_pose.keyframes[i].translation = base->keyframes[i].translation + secondary->keyframes[i].translation * t;
				m_pose.keyframes[i].rotation = base->keyframes[i].rotation * glm::slerp(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), secondary->keyframes[i].rotation, t);
				m_pose.keyframes[i].scale = base->keyframes[i].scale + secondary->keyframes[i].scale * t;
			}
			else
			{
				m_pose.keyframes[i].translation = base->keyframes[i].translation;
				m_pose.keyframes[i].rotation = base->keyframes[i].rotation;
				m_pose.keyframes[i].scale = base->keyframes[i].scale;
			}
		}
	}

	return &m_pose;
}