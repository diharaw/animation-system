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

	output_animation->channels.resize(skeleton->num_raw_bones());
	output_animation->name = std::string(animation->mName.C_Str());
	output_animation->duration = animation->mDuration / animation->mTicksPerSecond;
	output_animation->keyframe_count = animation->mChannels[0]->mNumPositionKeys;

	for (int i = 0; i < scene->mAnimations[0]->mNumChannels; i++)
	{
		aiNodeAnim* channel = scene->mAnimations[0]->mChannels[i];
		std::string channel_name = std::string(channel->mNodeName.C_Str());
		int joint_index = skeleton->find_joint_index(channel_name);

		if (joint_index != -1)
		{
			output_animation->channels[joint_index].joint_name = channel_name;
			output_animation->channels[joint_index].keyframes.resize(scene->mAnimations[0]->mChannels[0]->mNumPositionKeys);

			for (int j = 0; j < channel->mNumPositionKeys; j++)
			{
				output_animation->channels[joint_index].keyframes[j].translation = glm::vec3(channel->mPositionKeys[j].mValue.x,
					channel->mPositionKeys[j].mValue.y,
					channel->mPositionKeys[j].mValue.z);

				output_animation->channels[joint_index].keyframes[j].rotation = glm::quat(channel->mRotationKeys[j].mValue.w,
					channel->mRotationKeys[j].mValue.x,
					channel->mRotationKeys[j].mValue.y,
					channel->mRotationKeys[j].mValue.z);

				output_animation->channels[joint_index].keyframes[j].scale = glm::vec3(channel->mScalingKeys[j].mValue.x,
					channel->mScalingKeys[j].mValue.y,
					channel->mScalingKeys[j].mValue.z);
			}
		}
	}

	return output_animation;
}