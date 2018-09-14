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

}

Pose* AnimBlend::blend_additive(Pose* base, Pose* secondary, float t)
{
	for (uint32_t i = 0; i < base->num_keyframes; i++)
	{
		m_pose.keyframes[i].translation = base->keyframes[i].translation + secondary->keyframes[i].translation * t;
		m_pose.keyframes[i].rotation = glm::slerp(base->keyframes[i].rotation, secondary->keyframes[i].rotation, t);
		m_pose.keyframes[i].scale = base->keyframes[i].scale + secondary->keyframes[i].scale * t;
	}

	return &m_pose;
}

Pose* AnimBlend::blend_partial_additive(Pose* base, Pose* secondary, float t, const std::string& root_joint)
{

}