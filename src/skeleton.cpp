#include "skeleton.h"
#include <gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

Skeleton* Skeleton::create(const aiScene* scene)
{
	Skeleton* skeleton = new Skeleton();

	std::vector<aiBone*> temp_bone_list(256);

	skeleton->build_bone_list(scene->mRootNode, 0, scene, temp_bone_list);
	skeleton->m_joints.resize(skeleton->m_num_raw_bones);
	skeleton->build_skeleton(scene->mRootNode, 0, scene, temp_bone_list);

	return skeleton;
}

Skeleton::Skeleton()
{
	m_num_joints = 0;
	m_num_raw_bones = 0;
}

Skeleton::~Skeleton()
{

}

void Skeleton::build_bone_list(aiNode* node, int bone_index, const aiScene* scene, std::vector<aiBone*>& temp_bone_list)
{
	for (int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* current_mesh = scene->mMeshes[node->mMeshes[i]];

		for (int j = 0; j < current_mesh->mNumBones; j++)
		{
			temp_bone_list[bone_index] = current_mesh->mBones[j];
			bone_index++;
			m_num_raw_bones++;
		}
	}

	for (int i = 0; i < node->mNumChildren; i++)
		build_bone_list(node->mChildren[i], bone_index, scene, temp_bone_list);
}

void Skeleton::build_skeleton(aiNode* node, int bone_index, const aiScene* scene, std::vector<aiBone*>& temp_bone_list)
{
	std::string node_name = std::string(node->mName.C_Str());

	int count = bone_index;

	for (int i = 0; i < m_num_raw_bones; i++)
	{
		if (!temp_bone_list[i])
			continue;

		std::string bone_name = std::string(temp_bone_list[i]->mName.C_Str());

		if (bone_name == node_name)
		{
			m_joints[m_num_joints].name = std::string(temp_bone_list[i]->mName.C_Str());
			m_joints[m_num_joints].offset_transform = glm::transpose(glm::make_mat4(&temp_bone_list[i]->mOffsetMatrix.a1));

			aiNode* parent = node->mParent;
			int index;

			while (parent)
			{
				index = find_joint_index(std::string(parent->mName.C_Str()));

				if (index == -1)
					parent = parent->mParent;
				else
					break;
			}

			m_joints[m_num_joints].parent_index = index;
			m_num_joints++;
			break;
		}
	}

	for (int i = 0; i < node->mNumChildren; i++)
		build_skeleton(node->mChildren[i], m_num_joints, scene, temp_bone_list);
}

int32_t Skeleton::find_joint_index(const std::string& channel_name)
{
	for (int i = 0; i < m_num_raw_bones; i++)
	{
		if (m_joints[i].name == channel_name)
			return i;
	}

	return -1;
}