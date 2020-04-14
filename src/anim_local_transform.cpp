#include "anim_local_transform.h"
#include <gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>

AnimLocalTransform::AnimLocalTransform(Skeleton* skeleton) : m_skeleton(skeleton)
{
	for (int i = 0; i < MAX_BONES; i++)
		m_transforms.transforms[i] = glm::mat4(1.0f);
}

AnimLocalTransform::~AnimLocalTransform()
{

}

PoseTransforms* AnimLocalTransform::generate_transforms(Pose* pose)
{
	Joint* joints = m_skeleton->joints();

	for (uint32_t i = 0; i < m_skeleton->num_bones(); i++)
		m_transforms.transforms[i] = transform_from_keyframe(pose->keyframes[i]);

	return &m_transforms;
}


glm::mat4 AnimLocalTransform::transform_from_keyframe(const Keyframe& keyframe)
{
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), keyframe.translation);
	glm::mat4 rotation = glm::toMat4(keyframe.rotation);
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), keyframe.scale);

	glm::mat4 local_transform = translation * rotation; // * scale;

	return local_transform;
}