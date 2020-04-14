#pragma once

#include "skeletal_mesh.h"

class AnimLocalTransform
{
public:
	AnimLocalTransform(Skeleton* skeleton);
	~AnimLocalTransform();
	PoseTransforms* generate_transforms(Pose* pose);

private:
	glm::mat4 transform_from_keyframe(const Keyframe& keyframe);

private:
	Skeleton*	   m_skeleton;
	PoseTransforms m_transforms;
};
