#include "animation.h"
#include <logger.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "skeleton.h"

glm::vec3 translation_delta(const glm::vec3& reference, glm::vec3 additive)
{
	return additive - reference;
}

glm::vec3 scale_delta(const glm::vec3& reference, glm::vec3 additive)
{
	return additive / reference;
}

glm::quat rotation_delta(const glm::quat& reference, glm::quat additive)
{
	return glm::conjugate(reference) * additive;
}

Animation* Animation::load(const std::string& name, Skeleton* skeleton, bool additive)
{
	const aiScene* scene;
	Assimp::Importer importer;
	scene = importer.ReadFile(name, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

	if (!scene)
	{
		DW_LOG_ERROR("Failed to load animation file : " + name);
		return nullptr;
	}

	if (!scene->mAnimations)
	{
		DW_LOG_ERROR("No Animations available in file : " + name);
		return nullptr;
	}

	aiAnimation* animation = scene->mAnimations[0];

	Animation* output_animation = new Animation();

	output_animation->channels.resize(skeleton->num_bones());
	output_animation->name = std::string(animation->mName.C_Str());
	output_animation->duration = animation->mDuration / animation->mTicksPerSecond;
	output_animation->duration_in_ticks = animation->mDuration;
	output_animation->ticks_per_second = animation->mTicksPerSecond;
	output_animation->keyframe_count = animation->mChannels[0]->mNumPositionKeys;

	for (int i = 0; i < scene->mAnimations[0]->mNumChannels; i++)
	{
		aiNodeAnim* channel = scene->mAnimations[0]->mChannels[i];
		std::string channel_name = trimmed_name(channel->mNodeName.C_Str());

		int joint_index = skeleton->find_joint_index(channel_name);

		if (joint_index != -1)
		{
			output_animation->channels[joint_index].joint_name = channel_name;

			// Translation Keyframes
			output_animation->channels[joint_index].translation_keyframes.resize(channel->mNumPositionKeys);

			glm::vec3 reference_translation;

			if (channel->mNumPositionKeys > 0)
				reference_translation = glm::vec3(channel->mPositionKeys[0].mValue.x, channel->mPositionKeys[0].mValue.y, channel->mPositionKeys[0].mValue.z);

			for (int j = 0; j < channel->mNumPositionKeys; j++)
			{
				output_animation->channels[joint_index].translation_keyframes[j].time = channel->mPositionKeys[j].mTime;

				output_animation->channels[joint_index].translation_keyframes[j].translation = glm::vec3(channel->mPositionKeys[j].mValue.x,
					channel->mPositionKeys[j].mValue.y,
					channel->mPositionKeys[j].mValue.z);

				if (additive)
					output_animation->channels[joint_index].translation_keyframes[j].translation = translation_delta(reference_translation, output_animation->channels[joint_index].translation_keyframes[j].translation);
			}

			// Rotation Keyframes
			output_animation->channels[joint_index].rotation_keyframes.resize(channel->mNumRotationKeys);

			glm::quat reference_rotation;

			if (channel->mNumRotationKeys > 0)
			{
				reference_rotation = glm::quat(channel->mRotationKeys[0].mValue.w,
					channel->mRotationKeys[0].mValue.x,
					channel->mRotationKeys[0].mValue.y,
					channel->mRotationKeys[0].mValue.z);
			}

			for (int j = 0; j < channel->mNumRotationKeys; j++)
			{
				output_animation->channels[joint_index].rotation_keyframes[j].time = channel->mRotationKeys[j].mTime;

				output_animation->channels[joint_index].rotation_keyframes[j].rotation = glm::quat(channel->mRotationKeys[j].mValue.w,
					channel->mRotationKeys[j].mValue.x,
					channel->mRotationKeys[j].mValue.y,
					channel->mRotationKeys[j].mValue.z);

				if (additive)
					output_animation->channels[joint_index].rotation_keyframes[j].rotation = rotation_delta(reference_rotation, output_animation->channels[joint_index].rotation_keyframes[j].rotation);
			}

			// Scale Keyframes
			output_animation->channels[joint_index].scale_keyframes.resize(channel->mNumScalingKeys);

			glm::vec3 reference_scale;

			if (channel->mNumScalingKeys > 0)
				reference_scale = glm::vec3(channel->mScalingKeys[0].mValue.x, channel->mScalingKeys[0].mValue.y, channel->mScalingKeys[0].mValue.z);

			for (int j = 0; j < channel->mNumScalingKeys; j++)
			{
				output_animation->channels[joint_index].scale_keyframes[j].time = channel->mScalingKeys[j].mTime;

				output_animation->channels[joint_index].scale_keyframes[j].scale = glm::vec3(channel->mScalingKeys[j].mValue.x,
					channel->mScalingKeys[j].mValue.y,
					channel->mScalingKeys[j].mValue.z);

				if (additive)
					output_animation->channels[joint_index].scale_keyframes[j].scale = scale_delta(reference_scale, output_animation->channels[joint_index].scale_keyframes[j].scale);
			}
		}
	}

	return output_animation;
}

std::string trimmed_name(const std::string& name)
{
	size_t pos = name.find_first_of(':');

	if (pos != std::string::npos)
		return name.substr(pos + 1);
	else
		return name;
}