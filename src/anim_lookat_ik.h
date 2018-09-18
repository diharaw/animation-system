#pragma once

#include "skeletal_mesh.h"

class AnimLookAtIK
{
public:
	AnimLookAtIK(Skeleton* skeleton);
	~AnimLookAtIK();

	PoseTransforms* look_at(PoseTransforms* input, PoseTransforms* input_local, const glm::vec3& target, float max_angle, const std::string& joint);

private:
	Skeleton* m_skeleton;
	PoseTransforms m_transforms;
};