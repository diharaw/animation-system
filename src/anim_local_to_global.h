#pragma once

#include "skeletal_mesh.h"

class AnimLocalToGlobal
{
public:
	AnimLocalToGlobal(Skeleton* skeleton);
	~AnimLocalToGlobal();
	PoseTransforms* generate_transforms(Pose* pose);
	PoseTransforms* local_transforms();

private:
	glm::mat4 transform_from_keyframe(const Keyframe& keyframe);

private:
	Skeleton*	   m_skeleton;
	PoseTransforms m_transforms;
	PoseTransforms m_global_transforms;
	PoseTransforms m_local_transforms;
};
