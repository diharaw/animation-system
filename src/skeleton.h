#pragma once

#include "animation.h"

struct Joint
{
	std::string name;
	glm::mat4   global_transform;
	glm::mat4	offset_transform;
	int32_t		parent_index;
};

struct Skeleton
{
	uint16_t		   num_joints;
	std::vector<Joint> joints;
};