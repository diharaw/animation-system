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
	
	m_pose.num_keyframes = m_skeleton->num_bones();

	for (int i = 0; i < m_skeleton->num_bones(); i++)
	{
		const AnimationChannel& channel = m_animation->channels[i];

		Keyframe result;

		// Calculate interpolated translation
		{
			const uint32_t idx_1 = find_translation_key(channel.translation_keyframes, m_local_time);
			const uint32_t idx_2 = idx_1 + 1;

			float delta = (float)(channel.translation_keyframes[idx_2].time - channel.translation_keyframes[idx_1].time);
			float factor = (m_local_time - (float)channel.translation_keyframes[idx_1].time) / delta;

			result.translation = interpolate_translation(channel.translation_keyframes[idx_1].translation, channel.translation_keyframes[idx_2].translation, factor);
		}
		
		// Calculate interpolated rotation
		{
			const uint32_t idx_1 = find_rotation_key(channel.rotation_keyframes, m_local_time);
			const uint32_t idx_2 = idx_1 + 1;

			float delta = (float)(channel.rotation_keyframes[idx_2].time - channel.rotation_keyframes[idx_1].time);
			float factor = (m_local_time - (float)channel.rotation_keyframes[idx_1].time) / delta;

			result.rotation = interpolate_rotation(channel.rotation_keyframes[idx_1].rotation, channel.rotation_keyframes[idx_2].rotation, factor);
		}

		// Calculate interpolated scale
		{
			const uint32_t idx_1 = find_scale_key(channel.scale_keyframes, m_local_time);
			const uint32_t idx_2 = idx_1 + 1;

			float delta = (float)(channel.scale_keyframes[idx_2].time - channel.scale_keyframes[idx_1].time);
			float factor = (m_local_time - (float)channel.scale_keyframes[idx_1].time) / delta;

			result.scale = interpolate_scale(channel.scale_keyframes[idx_1].scale, channel.scale_keyframes[idx_2].scale, factor);
		}

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

glm::vec3 AnimSample::interpolate_translation(const glm::vec3& a, const glm::vec3& b, float t)
{
	return glm::lerp(a, b, t);
}

glm::vec3 AnimSample::interpolate_scale(const glm::vec3& a, const glm::vec3& b, float t)
{
	return glm::lerp(a, b, t);
}

glm::quat AnimSample::interpolate_rotation(const glm::quat& a, const glm::quat& b, float t)
{
	return glm::slerp(a, b, t);
}

uint32_t AnimSample::find_translation_key(const std::vector<TranslationKey>& translations, double ticks)
{
	uint32_t idx = 0;

	for (uint32_t i = 0; i < (translations.size() - 1); i++)
	{
		if (ticks < translations[i].time)
		{
			idx = i;
			break;
		}
	}

	return idx;
}

uint32_t AnimSample::find_rotation_key(const std::vector<RotationKey>& rotations, double ticks)
{
	uint32_t idx = 0;

	for (uint32_t i = 0; i < (rotations.size() - 1); i++)
	{
		if (ticks < rotations[i].time)
		{
			idx = i;
			break;
		}
	}

	return idx;
}

uint32_t AnimSample::find_scale_key(const std::vector<ScaleKey>& scale, double ticks)
{
	uint32_t idx = 0;

	for (uint32_t i = 0; i < (scale.size() - 1); i++)
	{
		if (ticks < scale[i].time)
		{
			idx = i;
			break;
		}
	}

	return idx;
}