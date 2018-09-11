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
	Keyframe interpolate(const Keyframe& a, const Keyframe& b, float t);

private:
	double	       m_global_time;
	double		   m_local_time;
	float		   m_local_time_normalized;
	Skeleton*	   m_skeleton;
	Animation*	   m_animation;
	float		   m_playback_rate;
	Pose		   m_pose;
};
