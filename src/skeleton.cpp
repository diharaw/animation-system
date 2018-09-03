#include "skeleton.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

Skeleton* Skeleton::create(const aiScene* scene)
{
	Skeleton* skeleton = new Skeleton();

	std::vector<aiBone> temp_bone_list(256);
	skeleton->build_bone_list()
}

Skeleton::Skeleton()
{
	num_joints = 0;
	num_raw_bones = 0;
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
			num_raw_bones++;
		}
	}

	for (int i = 0; i < node->mNumChildren; i++)
		build_bone_list(node->mChildren[i], bone_index, scene, temp_bone_list);
}

void Skeleton::build_skeleton(aiNode* node, int bone_index, const aiScene* scene)
{

}

int32_t Skeleton::find_joint_index(const std::string& channel_name)
{

}

Skeleton* build_skeleton(Skeleton* skeleton, aiNode* node, int bone_index, const aiScene* scene)
{
	std::string node_name = std::string(node->mName.C_Str());

	int count = bone_index;

	for (int i = 0; i < m_RawBoneCount; i++)
	{
		std::string boneName = std::string(m_TempBoneList[i].mName.C_Str());

		if (boneName == nodeName)
		{
			skeleton->m_Joints[m_ActualBoneCount].m_name = std::string(m_TempBoneList[i].mName.C_Str());
			skeleton->m_Joints[m_ActualBoneCount].m_OffsetTransform = glm::transpose(glm::make_mat4(&m_TempBoneList[i].mOffsetMatrix.a1));

			aiNode* parent = _node->mParent;
			int index;

			while (parent)
			{
				index = FindJointIndex(std::string(parent->mName.C_Str()));

				if (index == -1)
					parent = parent->mParent;
				else
					break;
			}

			m_Skeleton->m_Joints[m_ActualBoneCount].m_parentIndex = index;
			m_ActualBoneCount++;
			break;
		}
	}

	for (int i = 0; i < _node->mNumChildren; i++)
	{
		BuildSkeleton(_node->mChildren[i], m_ActualBoneCount, _scene);
	}
}

Skeleton* Skeleton::create(const aiScene* scene)
{
	Skeleton* skeleton = new Skeleton();

	skeleton->num_raw_bones = 0;

	std::vector<aiBone> temp_bone_list(256);

	build_bone_list(skeleton, scene->mRootNode, 0, scene, temp_bone_list);

	build_skeleton(skeleton, scene->mRootNode, 0, scene);
}