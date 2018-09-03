#include "skeletal_mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>


SkeletalMesh* SkeletalMesh::load(const std::string& name, Skeleton* skeleton)
{
	const aiScene* scene;
	Assimp::Importer importer;
	scene = importer.ReadFile(name, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

	SkeletalMesh* skeletal_mesh = new SkeletalMesh();

	if (skeleton)
		skeletal_mesh->m_skeleton = skeleton;
	else
		skeletal_mesh->m_skeleton = Skeleton::create(scene);

	skeletal_mesh->m_sub_meshes.resize(scene->mNumMeshes);

	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
	const aiColor4D Zero4D(0.0f, 0.0f, 0.0f, 0.0f);

	uint32_t num_vertices = 0;
	uint32_t num_indices = 0;

	skeletal_mesh->m_has_vertex_colors = scene->mMeshes[0]->HasVertexColors(0);

	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		skeletal_mesh->m_sub_meshes[i].num_indices = scene->mMeshes[i]->mNumFaces * 3;
		skeletal_mesh->m_sub_meshes[i].base_vertex = num_vertices;
		skeletal_mesh->m_sub_meshes[i].base_index = num_indices;

		num_vertices += scene->mMeshes[i]->mNumVertices;
		num_indices += skeletal_mesh->m_sub_meshes[i].num_indices;

		for (int j = 0; j < scene->mMeshes[i]->mNumVertices; j++)
		{
			const aiVector3D* pPos = &(scene->mMeshes[i]->mVertices[j]);
			const aiVector3D* pNormal = &(scene->mMeshes[i]->mNormals[j]);
			const aiVector3D* pTexCoord = scene->mMeshes[i]->HasTextureCoords(0) ? &(scene->mMeshes[i]->mTextureCoords[0][j]) : &Zero3D;
			const aiColor4D*  pColor = scene->mMeshes[i]->HasVertexColors(0) ? &(scene->mMeshes[i]->mColors[0][j]) : &Zero4D;

			if (skeletal_mesh->m_has_vertex_colors)
			{
				SkeletalColoredVertex vertex;
				vertex.position = glm::vec3(pPos->x, pPos->y, pPos->z);
				vertex.normal = glm::vec3(pNormal->x, pNormal->y, pNormal->z);
				vertex.texcoord = glm::vec2(pTexCoord->x, pTexCoord->y);
				vertex.color = glm::vec4(pColor->r, pColor->g, pColor->b, pColor->a);
				vertex.bone_indices = glm::ivec4(0, 0, 0, 0);
				vertex.bone_weights = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

				skeletal_mesh->m_color_vertices.push_back(vertex);
			}
			else
			{
				SkeletalVertex vertex;
				vertex.position = glm::vec3(pPos->x, pPos->y, pPos->z);
				vertex.normal = glm::vec3(pNormal->x, pNormal->y, pNormal->z);
				vertex.texcoord = glm::vec2(pTexCoord->x, pTexCoord->y);
				vertex.bone_indices = glm::ivec4(0, 0, 0, 0);
				vertex.bone_weights = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

				skeletal_mesh->m_vertices.push_back(vertex);
			}
		}

		for (int j = 0; j < scene->mMeshes[i]->mNumFaces; j++)
		{
			skeletal_mesh->m_indices.push_back(scene->mMeshes[i]->mFaces[j].mIndices[0]);
			skeletal_mesh->m_indices.push_back(scene->mMeshes[i]->mFaces[j].mIndices[1]);
			skeletal_mesh->m_indices.push_back(scene->mMeshes[i]->mFaces[j].mIndices[2]);
		}
	}

	for (int32_t i = 0; i < skeletal_mesh->m_sub_meshes.size(); i++)
	{
		for (uint32_t j = 0; j < scene->mMeshes[i]->mNumBones; j++)
		{
			std::string joint_name = std::string(scene->mMeshes[i]->mBones[j]->mName.C_Str());

			int joint_index = skeletal_mesh->m_skeleton->find_joint_index(joint_name);

			for (int k = 0; k < scene->mMeshes[i]->mBones[j]->mNumWeights; k++)
			{
				uint32_t VertexID = skeletal_mesh->m_sub_meshes[i].base_vertex + scene->mMeshes[i]->mBones[j]->mWeights[k].mVertexId;
				float Weight = scene->mMeshes[i]->mBones[j]->mWeights[k].mWeight;

				for (int x = 0; x < 4; x++) // hardcoded to 4, since vec4's only contain 4 elements
				{
					if (!skeletal_mesh->m_has_vertex_colors && skeletal_mesh->m_vertices[VertexID].bone_weights[x] == 0.0f)
					{
						skeletal_mesh->m_vertices[VertexID].bone_indices[x] = joint_index;
						skeletal_mesh->m_vertices[VertexID].bone_weights[x] = Weight;
						break;
					}
					else if (skeletal_mesh->m_has_vertex_colors && skeletal_mesh->m_color_vertices[VertexID].bone_weights[x] == 0.0f)
					{
						skeletal_mesh->m_color_vertices[VertexID].bone_indices[x] = joint_index;
						skeletal_mesh->m_color_vertices[VertexID].bone_weights[x] = Weight;
						break;
					}
				}
			}
		}
	}
}

SkeletalMesh::SkeletalMesh()
{

}

SkeletalMesh::~SkeletalMesh()
{

}

void SkeletalMesh::bind_vao()
{

}