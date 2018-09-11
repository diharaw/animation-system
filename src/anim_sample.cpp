#include "anim_sample.h"
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/compatibility.hpp>

AnimSample::AnimSample(Skeleton* skeleton, Animation* animation) : m_skeleton(skeleton), m_animation(animation), m_playback_rate(1.0f), m_global_time(0.0)
{

}

AnimSample::~AnimSample()
{

}

Pose* AnimSample::sample(double dt)
{
	m_global_time += (dt * m_playback_rate); // dt is Delta Time in seconds.

	float ticks_per_second = (float)(m_animation->ticks_per_second != 0 ? m_animation->ticks_per_second : 25.0f);
	float time_in_ticks = ticks_per_second * m_global_time; 
	m_local_time = fmod(time_in_ticks, m_animation->duration_in_ticks);
	m_local_time_normalized = static_cast<float>(m_local_time) / static_cast<float>(m_animation->duration_in_ticks);
	
	float int_part = 0;
	float lerp_factor = modf(m_local_time_normalized * static_cast<float>(m_animation->keyframe_count), &int_part);

	int keyframe_1_idx = m_local_time_normalized * m_animation->keyframe_count;
	int keyframe_2_idx = (keyframe_1_idx + 1) % m_animation->keyframe_count;

	m_pose.num_keyframes = m_skeleton->num_bones();

	for (int i = 0; i < m_skeleton->num_bones(); i++)
	{
		const Keyframe& keyframe_1 = m_animation->channels[i].keyframes[keyframe_1_idx];
		const Keyframe& keyframe_2 = m_animation->channels[i].keyframes[keyframe_2_idx];

		Keyframe result = interpolate(keyframe_1, keyframe_2, lerp_factor);

		m_pose.keyframes[i] = result;
	}

	return &m_pose;
}

void AnimSample::set_playback_rate(float rate)
{
	if (rate < 0.0f || rate > 1.0f)
		return;

	m_playback_rate = rate;
}

float AnimSample::playback_rate()
{
	return m_playback_rate;
}

Keyframe AnimSample::interpolate(const Keyframe& a, const Keyframe& b, float t)
{
	Keyframe result;

	result.translation = glm::lerp(a.translation, b.translation, t);
	result.scale = glm::lerp(a.scale, b.scale, t);
	result.rotation = glm::slerp(a.rotation, b.rotation, t);

	return result;
}