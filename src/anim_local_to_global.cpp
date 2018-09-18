#include "anim_local_to_global.h"
#include <gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>

AnimLocalToGlobal::AnimLocalToGlobal(Skeleton* skeleton) : m_skeleton(skeleton)
{
	for (int i = 0; i < MAX_BONES; i++)
		m_transforms.transforms[i] = glm::mat4(1.0f);
}

AnimLocalToGlobal::~AnimLocalToGlobal()
{

}

PoseTransforms* AnimLocalToGlobal::generate_transforms(Pose* pose)
{
	Joint* joints = m_skeleton->joints();

	for (uint32_t i = 0; i < pose->num_keyframes; i++)
	{
		m_local_transforms.transforms[i] = transform_from_keyframe(pose->keyframes[i]);

		if (joints[i].parent_index == -1)
			m_global_transforms.transforms[i] = m_local_transforms.transforms[i];
		else
			m_global_transforms.transforms[i] = m_global_transforms.transforms[joints[i].parent_index] * m_local_transforms.transforms[i];
	}
	
	return &m_global_transforms;
}

PoseTransforms* AnimLocalToGlobal::local_transforms()
{
	return &m_local_transforms;
}

glm::mat4 AnimLocalToGlobal::transform_from_keyframe(const Keyframe& keyframe)
{
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), keyframe.translation);
	glm::mat4 rotation = glm::toMat4(keyframe.rotation);
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), keyframe.scale);

	glm::mat4 local_transform = translation * rotation; // * scale;

	return local_transform;
}