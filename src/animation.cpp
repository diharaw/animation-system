#include "animation.h"
#include <logger.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "skeleton.h"

Animation* Animation::load(const std::string& name, Skeleton* skeleton)
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
		std::string channel_name = std::string(channel->mNodeName.C_Str());

		size_t pos = channel_name.find_first_of(':');

		if (pos != std::string::npos)
			channel_name = channel_name.substr(pos + 1);

		int joint_index = skeleton->find_joint_index(channel_name);

		if (joint_index != -1)
		{
			output_animation->channels[joint_index].joint_name = channel_name;

			// Translation Keyframes
			output_animation->channels[joint_index].translation_keyframes.resize(channel->mNumPositionKeys);

			for (int j = 0; j < channel->mNumPositionKeys; j++)
			{
				output_animation->channels[joint_index].translation_keyframes[j].time = channel->mPositionKeys[j].mTime;

				output_animation->channels[joint_index].translation_keyframes[j].translation = glm::vec3(channel->mPositionKeys[j].mValue.x,
					channel->mPositionKeys[j].mValue.y,
					channel->mPositionKeys[j].mValue.z);
			}

			// Rotation Keyframes
			output_animation->channels[joint_index].rotation_keyframes.resize(channel->mNumRotationKeys);

			for (int j = 0; j < channel->mNumRotationKeys; j++)
			{
				output_animation->channels[joint_index].rotation_keyframes[j].time = channel->mRotationKeys[j].mTime;

				output_animation->channels[joint_index].rotation_keyframes[j].rotation = glm::quat(channel->mRotationKeys[j].mValue.w,
					channel->mRotationKeys[j].mValue.x,
					channel->mRotationKeys[j].mValue.y,
					channel->mRotationKeys[j].mValue.z);
			}

			// Scale Keyframes
			output_animation->channels[joint_index].scale_keyframes.resize(channel->mNumScalingKeys);

			for (int j = 0; j < channel->mNumScalingKeys; j++)
			{
				output_animation->channels[joint_index].scale_keyframes[j].time = channel->mPositionKeys[j].mTime;

				output_animation->channels[joint_index].scale_keyframes[j].scale = glm::vec3(channel->mScalingKeys[j].mValue.x,
					channel->mScalingKeys[j].mValue.y,
					channel->mScalingKeys[j].mValue.z);
			}
		}
	}

	return output_animation;
}