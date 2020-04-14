#include "anim_global_transform.h"

AnimGlobalTransform::AnimGlobalTransform(Skeleton* skeleton) : m_skeleton(skeleton)
{
	for (int i = 0; i < MAX_BONES; i++)
		m_transforms.transforms[i] = glm::mat4(1.0f);
}

AnimGlobalTransform::~AnimGlobalTransform()
{

}

PoseTransforms* AnimGlobalTransform::generate_transforms(PoseTransforms* local_transforms)
{
	Joint* joints = m_skeleton->joints();

	for (uint32_t i = 0; i < m_skeleton->num_bones(); i++)
	{
		if (joints[i].parent_index == -1)
			m_transforms.transforms[i] = local_transforms->transforms[i];
		else
			m_transforms.transforms[i] = m_transforms.transforms[joints[i].parent_index] * local_transforms->transforms[i];
	}
	
	return &m_transforms;
}