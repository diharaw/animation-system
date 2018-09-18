#pragma once

#include "skeletal_mesh.h"

class AnimFabrikIK
{
public:
	AnimFabrikIK(Skeleton* skeleton);
	~AnimFabrikIK();

	PoseTransforms* fabrik(PoseTransforms* input, const glm::vec3& end_effector, const std::string& root);

private:
	PoseTransforms m_transforms;
};