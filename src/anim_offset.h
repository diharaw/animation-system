#pragma once

#include "skeletal_mesh.h"

class AnimOffset
{
public:
	AnimOffset(Skeleton* skeleton);
	~AnimOffset();
	PoseTransforms* offset(PoseTransforms* transforms);

private:
	Skeleton*	   m_skeleton;
	PoseTransforms m_transforms;
};
