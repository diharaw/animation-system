#pragma once

#include "skeletal_mesh.h"

class AnimGlobalTransform
{
public:
	AnimGlobalTransform(Skeleton* skeleton);
	~AnimGlobalTransform();
	PoseTransforms* generate_transforms(PoseTransforms* local_transforms);

private:
	Skeleton*	   m_skeleton;
	PoseTransforms m_transforms;
};
