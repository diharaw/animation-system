#pragma once

#include "skeletal_mesh.h"

class AnimSample
{
public:
	AnimSample(Skeleton* skeleton, Animation* animation);
	~AnimSample();
	Pose* sample(double dt);
	void set_playback_rate(float rate);
	float playback_rate();

private:
	glm::vec3 interpolate_translation(const glm::vec3& a, const glm::vec3& b, float t);
	glm::vec3 interpolate_scale(const glm::vec3& a, const glm::vec3& b, float t);
	glm::quat interpolate_rotation(const glm::quat& a, const glm::quat& b, float t);
	uint32_t find_translation_key(const std::vector<TranslationKey>& translations, double ticks);
	uint32_t find_rotation_key(const std::vector<RotationKey>& rotations, double ticks);
	uint32_t find_scale_key(const std::vector<ScaleKey>& scale, double ticks);

private:
	double	       m_global_time;
	double		   m_local_time;
	float		   m_local_time_normalized;
	Skeleton*	   m_skeleton;
	Animation*	   m_animation;
	float		   m_playback_rate;
	Pose		   m_pose;
};
