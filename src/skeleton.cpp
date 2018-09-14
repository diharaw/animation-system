#include "skeleton.h"
#include <gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <iostream>

void print_scene_heirarchy(aiNode* node)
{
	if (node)
	{
		std::cout << node->mName.C_Str() << std::endl;

		for (int i = 0; i < node->mNumChildren; i++)
			print_scene_heirarchy(node->mChildren[i]);
	}
}

Skeleton* Skeleton::create(const aiScene* scene)
{
	Skeleton* skeleton = new Skeleton();

	std::vector<aiBone*> temp_bone_list(256);
	std::unordered_set<std::string> bone_map;

	std::cout << "\nBegin Print Scene\n" << std::endl;
	print_scene_heirarchy(scene->mRootNode);
	std::cout << "\nEnd Print Scene\n" << std::endl;

	skeleton->build_bone_list(scene->mRootNode, scene, temp_bone_list, bone_map);
	skeleton->m_joints.reserve(skeleton->m_num_joints);
	skeleton->build_skeleton(scene->mRootNode, 0, scene, temp_bone_list);

	std::cout << "\nBegin Print Joint List\n" << std::endl;
	
	for (int i = 0; i < skeleton->m_joints.size(); i++)
	{
		auto& joint = skeleton->m_joints[i];
		std::cout << "Index: " << i << ", Name: " << joint.name << ", Parent: " << joint.parent_index << std::endl;
	}

	std::cout << "\nEnd Print Joint List\n" << std::endl;

	return skeleton;
}

Skeleton::Skeleton()
{
	m_num_joints = 0;
}

Skeleton::~Skeleton()
{

}

void Skeleton::build_bone_list(aiNode* node, const aiScene* scene, std::vector<aiBone*>& temp_bone_list, std::unordered_set<std::string>& bone_map)
{
	for (int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* current_mesh = scene->mMeshes[node->mMeshes[i]];

		for (int j = 0; j < current_mesh->mNumBones; j++)
		{
			std::string bone_name = std::string(current_mesh->mBones[j]->mName.C_Str());

			if (bone_map.find(bone_name) == bone_map.end())
			{
				temp_bone_list[m_num_joints] = current_mesh->mBones[j];
				m_num_joints++;

				bone_map.insert(bone_name);
			}
		}
	}

	for (int i = 0; i < node->mNumChildren; i++)
		build_bone_list(node->mChildren[i], scene, temp_bone_list, bone_map);
}

void Skeleton::build_skeleton(aiNode* node, int bone_index, const aiScene* scene, std::vector<aiBone*>& temp_bone_list)
{
	std::string node_name = trimmed_name(node->mName.C_Str());

	int count = bone_index;

	for (int i = 0; i < m_num_joints; i++)
	{
		std::string bone_name = trimmed_name(temp_bone_list[i]->mName.C_Str());

		if (bone_name == node_name)
		{
			Joint joint;

			joint.name = bone_name;
			joint.offset_transform = glm::transpose(glm::make_mat4(&temp_bone_list[i]->mOffsetMatrix.a1));

			aiNode* parent = node->mParent;
			int index;

			while (parent)
			{
				index = find_joint_index(trimmed_name(parent->mName.C_Str()));

				if (index == -1)
					parent = parent->mParent;
				else
					break;
			}

			joint.parent_index = index;
			m_joints.push_back(joint);

			break;
		}
	}

	for (int i = 0; i < node->mNumChildren; i++)
		build_skeleton(node->mChildren[i], m_num_joints, scene, temp_bone_list);
}

int32_t Skeleton::find_joint_index(const std::string& channel_name)
{
	for (int i = 0; i < m_joints.size(); i++)
	{
		if (m_joints[i].name == channel_name)
			return i;
	}

	return -1;
}