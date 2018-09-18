#include "anim_offset.h"

AnimOffset::AnimOffset(Skeleton* skeleton) : m_skeleton(skeleton)
{

}

AnimOffset::~AnimOffset()
{

}

PoseTransforms* AnimOffset::offset(PoseTransforms* transforms)
{
	Joint* joints = m_skeleton->joints();

	for (uint32_t i = 0; i < m_skeleton->num_bones(); i++)
		m_transforms.transforms[i] = (transforms->transforms[i] * joints[i].offset_transform);

	return &m_transforms;
}