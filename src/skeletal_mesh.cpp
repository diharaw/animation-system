#include "skeletal_mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>
#include "logger.h"

SkeletalMesh* SkeletalMesh::load(const std::string& name, Skeleton* skeleton)
{
	const aiScene* scene;
	Assimp::Importer importer;
	scene = importer.ReadFile(name, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

	if (!scene)
	{
		DW_LOG_ERROR("Failed to load Skeletal Mesh in file : " + name);
		return nullptr;
	}

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
				}
			}
		}
	}

	//int count = 0;

	//for (auto& v : skeletal_mesh->m_vertices)
	//{
	//	if (v.bone_indices.x == 9999 || v.bone_indices.y == 9999 || v.bone_indices.z == 9999 || v.bone_indices.w == 9999)
	//		count++;

	//	std::cout << "Bone ID: " << v.bone_indices.x << ", " << v.bone_indices.y << ", " << v.bone_indices.z << ", " << v.bone_indices.w << std::endl;
	//	std::cout << "Bone Weights: " << v.bone_weights.x << ", " << v.bone_weights.y << ", " << v.bone_weights.z << ", " << v.bone_weights.w << std::endl;
	//}

	skeletal_mesh->create_gpu_objects();

	return skeletal_mesh;
}

SkeletalMesh::SkeletalMesh()
{

}

SkeletalMesh::~SkeletalMesh()
{

}

void SkeletalMesh::bind_vao()
{
	m_vao->bind();
}

void SkeletalMesh::create_gpu_objects()
{
	if (m_has_vertex_colors)
	{
		// Create vertex buffer.
		m_vbo = std::make_unique<dw::VertexBuffer>(GL_STATIC_DRAW, sizeof(SkeletalColoredVertex) * m_color_vertices.size(), &m_color_vertices[0]);

		if (!m_vbo)
			DW_LOG_ERROR("Failed to create Vertex Buffer");
	}
	else
	{
		// Create vertex buffer.
		m_vbo = std::make_unique<dw::VertexBuffer>(GL_STATIC_DRAW, sizeof(SkeletalVertex) * m_vertices.size(), &m_vertices[0]);

		if (!m_vbo)
			DW_LOG_ERROR("Failed to create Index Buffer");
	}

	// Create index buffer.
	m_ibo = std::make_unique<dw::IndexBuffer>(GL_STATIC_DRAW, sizeof(uint32_t) * m_indices.size(), &m_indices[0]);

	if (!m_ibo)
		DW_LOG_ERROR("Failed to create Index Buffer");

	if (m_has_vertex_colors)
	{
		// Declare vertex attributes.
		dw::VertexAttrib attribs[] =
		{
			{ 3, GL_FLOAT, false, 0 },
			{ 2, GL_FLOAT, false, offsetof(SkeletalColoredVertex, texcoord) },
			{ 3, GL_FLOAT, false, offsetof(SkeletalColoredVertex, normal) },
			{ 4, GL_FLOAT, false, offsetof(SkeletalColoredVertex, color) },
			{ 4, GL_INT, false, offsetof(SkeletalColoredVertex, bone_indices) },
			{ 4, GL_FLOAT, false, offsetof(SkeletalColoredVertex, bone_weights) }
		};

		// Create vertex array.
		m_vao = std::make_unique<dw::VertexArray>(m_vbo.get(), m_ibo.get(), sizeof(SkeletalColoredVertex), 6, attribs);

		if (!m_vao)
			DW_LOG_ERROR("Failed to create Vertex Array");
	}
	else
	{
		// Declare vertex attributes.
		dw::VertexAttrib attribs[] =
		{
			{ 3, GL_FLOAT, false, 0 },
			{ 2, GL_FLOAT, false, offsetof(SkeletalVertex, texcoord) },
			{ 3, GL_FLOAT, false, offsetof(SkeletalVertex, normal) },
			{ 3, GL_FLOAT, false, offsetof(SkeletalVertex, tangent) },
			{ 4, GL_INT, false, offsetof(SkeletalVertex, bone_indices) },
			{ 4, GL_FLOAT, false, offsetof(SkeletalVertex, bone_weights) }
		};

		// Create vertex array.
		m_vao = std::make_unique<dw::VertexArray>(m_vbo.get(), m_ibo.get(), sizeof(SkeletalVertex), 6, attribs);

		if (!m_vao)
			DW_LOG_ERROR("Failed to create Vertex Array");
	}
}