#pragma once

#include "skeletal_mesh.h"

class AnimBlend
{
public:
	AnimBlend(Skeleton* skeleton);
	~AnimBlend();

	Pose* blend(Pose* base, Pose* secondary, float t);
	Pose* blend_partial(Pose* base, Pose* secondary, float t, const std::string& root_joint);
	Pose* blend_additive(Pose* base, Pose* secondary, float t);
	Pose* blend_partial_additive(Pose* base, Pose* secondary, float t, const std::string& root_joint);
	Pose* blend_additive_with_reference(Pose* reference, Pose* secondary, float t);
	Pose* blend_partial_additive_with_reference(Pose* reference, Pose* secondary, float t, const std::string& root_joint);

private:
	Skeleton* m_skeleton;
	Pose	  m_pose;
};