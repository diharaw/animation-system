#include <application.h>
#include <mesh.h>
#include <camera.h>
#include <material.h>
#include <memory>
#include <iostream>
#include <stack>
#include "skeletal_mesh.h"
#include "anim_sample.h"
#include "anim_bone_to_local.h"
#include "anim_blend.h"
#include "blendspace_1d.h"

// Uniform buffer data structure.
struct ObjectUniforms
{
	DW_ALIGNED(16) glm::mat4 model;
};

struct GlobalUniforms
{
    DW_ALIGNED(16) glm::mat4 view;
    DW_ALIGNED(16) glm::mat4 projection;
};

struct BoneVertex
{
	glm::vec3 position;
	glm::vec3 normal;
};

#define CAMERA_FAR_PLANE 10000.0f

class AnimationStateMachine : public dw::Application
{
protected:
    
    // -----------------------------------------------------------------------------------------------------------------------------------
    
	bool init(int argc, const char* argv[]) override
	{
		// Create GPU resources.
		if (!create_shaders())
			return false;

		if (!create_bone_mesh())
			return false;

		if (!create_uniform_buffer())
			return false;

		// Load mesh.
		if (!load_mesh())
			return false;

		// Load animations.
		if (!load_animations())
			return false;

		// Create camera.
		create_camera();

		for (int i = 0; i < MAX_BONES; i++)
			m_pose_transforms.transforms[i] = glm::mat4(1.0f);

		m_index_stack.reserve(256);
		m_joint_pos.reserve(256);

		return true;
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	void update(double delta) override
	{
        // Debug GUI
        gui();
        
		// Update camera.
        update_camera();

		// Update global uniforms.
		Joint* joints = m_skeletal_mesh->skeleton()->joints();

		for (int i = 0; i < m_skeletal_mesh->skeleton()->num_bones(); i++)
			m_pose_transforms.transforms[i] = glm::inverse(joints[i].offset_transform);
		
		update_global_uniforms(m_global_uniforms);
		update_object_uniforms(m_character_transforms);

		// Bind and set viewport.
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, m_width, m_height);

		// Clear default framebuffer.
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Bind states.
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		// Update Skeleton
		update_animations();
	
        // Render Mesh.
		if (m_visualize_mesh)
			render_skeletal_meshes();

		// Render Joints.
		if (m_visualize_joints)
			visualize_skeleton(m_skeletal_mesh->skeleton());

		// Render Bones.
		if (m_visualize_bones)
			visualize_bones(m_skeletal_mesh->skeleton());
        
        // Render debug draw.
        m_debug_draw.render(nullptr, m_width, m_height, m_debug_mode ? m_debug_camera->m_view_projection : m_main_camera->m_view_projection);
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	void shutdown() override
	{

	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	void window_resized(int width, int height) override
	{
		// Override window resized method to update camera projection.
		m_main_camera->update_projection(60.0f, 0.1f, CAMERA_FAR_PLANE, float(m_width) / float(m_height));
        m_debug_camera->update_projection(60.0f, 0.1f, CAMERA_FAR_PLANE * 2.0f, float(m_width) / float(m_height));
	}

	// -----------------------------------------------------------------------------------------------------------------------------------
    
    void key_pressed(int code) override
    {
        // Handle forward movement.
        if(code == GLFW_KEY_W)
            m_heading_speed = m_camera_speed;
        else if(code == GLFW_KEY_S)
            m_heading_speed = -m_camera_speed;
        
        // Handle sideways movement.
        if(code == GLFW_KEY_A)
            m_sideways_speed = -m_camera_speed;
        else if(code == GLFW_KEY_D)
            m_sideways_speed = m_camera_speed;
    }
    
    // -----------------------------------------------------------------------------------------------------------------------------------
    
    void key_released(int code) override
    {
        // Handle forward movement.
        if(code == GLFW_KEY_W || code == GLFW_KEY_S)
            m_heading_speed = 0.0f;
        
        // Handle sideways movement.
        if(code == GLFW_KEY_A || code == GLFW_KEY_D)
            m_sideways_speed = 0.0f;
    }
    
    // -----------------------------------------------------------------------------------------------------------------------------------

	void mouse_pressed(int code) override
	{
		// Enable mouse look.
		if (code == GLFW_MOUSE_BUTTON_RIGHT)
			m_mouse_look = true;
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	void mouse_released(int code) override
	{
		// Disable mouse look.
		if (code == GLFW_MOUSE_BUTTON_RIGHT)
			m_mouse_look = false;
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

protected:

	// -----------------------------------------------------------------------------------------------------------------------------------

	dw::AppSettings intial_app_settings() override
	{
		dw::AppSettings settings;

		settings.resizable = true;
		settings.maximized = false;
		settings.refresh_rate = 60;
		settings.major_ver = 4;
		settings.width = 1280;
		settings.height = 720;
		settings.title = "Animation State Machine Demo";

		return settings;
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

private:
    
	// -----------------------------------------------------------------------------------------------------------------------------------

	bool create_bone_mesh()
	{
		const float kInter = 0.2f;
		const float kScale = 1.0f;

		const glm::vec3 pos[6] = 
		{
			glm::vec3(1.f, 0.f, 0.f) * kScale,     glm::vec3(kInter, .1f, .1f) * kScale,
			glm::vec3(kInter, .1f, -.1f) * kScale, glm::vec3(kInter, -.1f, -.1f) * kScale,
			glm::vec3(kInter, -.1f, .1f) * kScale, glm::vec3(0.f, 0.f, 0.f) * kScale
		};

		const glm::vec3 normals[8] =
		{
			glm::normalize(glm::cross(pos[2] - pos[1], pos[2] - pos[0])),
			glm::normalize(glm::cross(pos[1] - pos[2], pos[1] - pos[5])),
			glm::normalize(glm::cross(pos[3] - pos[2], pos[3] - pos[0])),
			glm::normalize(glm::cross(pos[2] - pos[3], pos[2] - pos[5])),
			glm::normalize(glm::cross(pos[4] - pos[3], pos[4] - pos[0])),
			glm::normalize(glm::cross(pos[3] - pos[4], pos[3] - pos[5])),
			glm::normalize(glm::cross(pos[1] - pos[4], pos[1] - pos[0])),
			glm::normalize(glm::cross(pos[4] - pos[1], pos[4] - pos[5])) 
		};

		const BoneVertex bones[24] = 
		{
			{pos[0], normals[0]}, {pos[2], normals[0]},
			{pos[1], normals[0]}, {pos[5], normals[1]},
			{pos[1], normals[1]}, {pos[2], normals[1]},
			{pos[0], normals[2]}, {pos[3], normals[2]},
			{pos[2], normals[2]}, {pos[5], normals[3]},
			{pos[2], normals[3]}, {pos[3], normals[3]},
			{pos[0], normals[4]}, {pos[4], normals[4]},
			{pos[3], normals[4]}, {pos[5], normals[5]},
			{pos[3], normals[5]}, {pos[4], normals[5]},
			{pos[0], normals[6]}, {pos[1], normals[6]},
			{pos[4], normals[6]}, {pos[5], normals[7]},
			{pos[4], normals[7]}, {pos[1], normals[7]} 
		};

		m_bone_vbo = std::make_unique<dw::VertexBuffer>(GL_STATIC_DRAW, sizeof(BoneVertex) * 24, (BoneVertex*)&bones[0]);

		if (!m_bone_vbo)
		{
			DW_LOG_ERROR("Failed to create Vertex Buffer");
			return false;
		}

		dw::VertexAttrib attribs[] =
		{
			{ 3, GL_FLOAT, false, 0 },
			{ 3, GL_FLOAT, false, offsetof(BoneVertex, normal) }
		};

		// Create vertex array.
		m_bone_vao = std::make_unique<dw::VertexArray>(m_bone_vbo.get(), nullptr, sizeof(BoneVertex), 2, attribs);

		if (!m_bone_vao)
		{
			DW_LOG_ERROR("Failed to create Vertex Array");
			return false;
		}

		return true;
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	bool create_shaders()
	{
		// Create general shaders
		m_vs = std::unique_ptr<dw::Shader>(dw::Shader::create_from_file(GL_VERTEX_SHADER, "shader/vs.glsl"));
		m_fs = std::unique_ptr<dw::Shader>(dw::Shader::create_from_file(GL_FRAGMENT_SHADER, "shader/fs.glsl"));

		if (!m_vs || !m_fs)
		{
			DW_LOG_FATAL("Failed to create Shaders");
			return false;
		}

		// Create general shader program
        dw::Shader* shaders[] = { m_vs.get(), m_fs.get() };
        m_program = std::make_unique<dw::Program>(2, shaders);

		if (!m_program)
		{
			DW_LOG_FATAL("Failed to create Shader Program");
			return false;
		}
        
        m_program->uniform_block_binding("u_GlobalUBO", 0);
        m_program->uniform_block_binding("u_ObjectUBO", 1);
        
        // Create Animation shaders
		m_anim_vs = std::unique_ptr<dw::Shader>(dw::Shader::create_from_file(GL_VERTEX_SHADER, "shader/skinning_vs.glsl"));
		m_anim_fs = std::unique_ptr<dw::Shader>(dw::Shader::create_from_file(GL_FRAGMENT_SHADER, "shader/skinning_fs.glsl"));

        if (!m_anim_vs || !m_anim_fs)
        {
            DW_LOG_FATAL("Failed to create Animation Shaders");
            return false;
        }
        
        // Create Animation shader program
        dw::Shader* anim_shaders[] = { m_anim_vs.get(), m_anim_fs.get() };
        m_anim_program = std::make_unique<dw::Program>(2, anim_shaders);
        
        if (!m_anim_program)
        {
            DW_LOG_FATAL("Failed to create Animation Shader Program");
            return false;
        }
        
		m_anim_program->uniform_block_binding("u_GlobalUBO", 0);
		m_anim_program->uniform_block_binding("u_ObjectUBO", 1);
		m_anim_program->uniform_block_binding("u_BoneUBO", 2);

		// Create Bone shaders
		m_bone_vs = std::unique_ptr<dw::Shader>(dw::Shader::create_from_file(GL_VERTEX_SHADER, "shader/bone_vs.glsl"));
		m_bone_fs = std::unique_ptr<dw::Shader>(dw::Shader::create_from_file(GL_FRAGMENT_SHADER, "shader/bone_fs.glsl"));

		if (!m_bone_vs || !m_bone_fs)
		{
			DW_LOG_FATAL("Failed to create Bone Shaders");
			return false;
		}

		// Create Bone shader program
		dw::Shader* bone_shaders[] = { m_bone_vs.get(), m_bone_fs.get() };
		m_bone_program = std::make_unique<dw::Program>(2, bone_shaders);

		if (!m_bone_program)
		{
			DW_LOG_FATAL("Failed to create Bone Shader Program");
			return false;
		}

		m_bone_program->uniform_block_binding("u_GlobalUBO", 0);
		m_bone_program->uniform_block_binding("u_ObjectUBO", 1);
		m_bone_program->uniform_block_binding("u_BoneUBO", 2);

		return true;
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	bool create_uniform_buffer()
	{
		// Create uniform buffer for object matrix data
        m_object_ubo = std::make_unique<dw::UniformBuffer>(GL_DYNAMIC_DRAW, sizeof(ObjectUniforms));
        
        // Create uniform buffer for global data
        m_global_ubo = std::make_unique<dw::UniformBuffer>(GL_DYNAMIC_DRAW, sizeof(GlobalUniforms));
        
        // Create uniform buffer for CSM data
		m_bone_ubo = std::make_unique<dw::UniformBuffer>(GL_DYNAMIC_DRAW, sizeof(PoseTransforms));

		return true;
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	bool load_mesh()
	{
		m_skeletal_mesh = std::unique_ptr<SkeletalMesh>(SkeletalMesh::load("mesh/Rifle/Rifle_Walk_Fwd.fbx"));

		if (!m_skeletal_mesh)
		{
			DW_LOG_FATAL("Failed to load mesh!");
			return false;
		}

		return true;
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	bool load_animations()
	{
		m_walk_animation = std::unique_ptr<Animation>(Animation::load("mesh/Rifle/Rifle_Walk_Fwd.fbx", m_skeletal_mesh->skeleton()));

		if (!m_walk_animation)
		{
			DW_LOG_FATAL("Failed to load animation!");
			return false;
		}

		m_jog_animation = std::unique_ptr<Animation>(Animation::load("mesh/Rifle/Rifle_Run_Fwd.fbx", m_skeletal_mesh->skeleton()));

		if (!m_jog_animation)
		{
			DW_LOG_FATAL("Failed to load animation!");
			return false;
		}

		m_run_animation = std::unique_ptr<Animation>(Animation::load("mesh/Rifle/Rifle_Sprint_Fwd.fbx", m_skeletal_mesh->skeleton()));

		if (!m_run_animation)
		{
			DW_LOG_FATAL("Failed to load animation!");
			return false;
		}

		m_additive_animation = std::unique_ptr<Animation>(Animation::load("mesh/Rifle/Aim Offsets/Rifle_Aim_Up.fbx", m_skeletal_mesh->skeleton()));

		if (!m_additive_animation)
		{
			DW_LOG_FATAL("Failed to load animation!");
			return false;
		}

		m_walk_sampler = std::make_unique<AnimSample>(m_skeletal_mesh->skeleton(), m_walk_animation.get());
		m_run_sampler = std::make_unique<AnimSample>(m_skeletal_mesh->skeleton(), m_run_animation.get());
		m_additive_sampler = std::make_unique<AnimSample>(m_skeletal_mesh->skeleton(), m_additive_animation.get());
		m_bone_to_local = std::make_unique<AnimBoneToLocal>(m_skeletal_mesh->skeleton());
		m_blend = std::make_unique<AnimBlend>(m_skeletal_mesh->skeleton());

		std::vector<Blendspace1D::Node*> nodes = {
			new Blendspace1D::Node(m_skeletal_mesh->skeleton(), m_walk_animation.get(), 0.0f),
			new Blendspace1D::Node(m_skeletal_mesh->skeleton(), m_jog_animation.get(), 50.0f),
			new Blendspace1D::Node(m_skeletal_mesh->skeleton(), m_run_animation.get(), 100.0f)
		};

		m_blendspace_1d = std::make_unique<Blendspace1D>(m_skeletal_mesh->skeleton(), nodes);

		return true;
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	void create_camera()
	{
        m_main_camera = std::make_unique<dw::Camera>(60.0f, 0.1f, CAMERA_FAR_PLANE, float(m_width) / float(m_height), glm::vec3(0.0f, 5.0f, 20.0f), glm::vec3(0.0f, 0.0, -1.0f));
        m_debug_camera = std::make_unique<dw::Camera>(60.0f, 0.1f, CAMERA_FAR_PLANE * 2.0f, float(m_width) / float(m_height), glm::vec3(0.0f, 5.0f, 20.0f), glm::vec3(0.0f, 0.0, -1.0f));
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	void render_skeleton(Skeleton* skeleton)
	{
		glClear(GL_DEPTH_BUFFER_BIT);

		Joint* joints = skeleton->joints();

		m_bone_program->use();

		// Bind uniform buffers.
		m_global_ubo->bind_base(0);
		m_object_ubo->bind_base(1);
		m_bone_ubo->bind_base(2);

		m_bone_vao->bind();

		glDrawArraysInstanced(GL_TRIANGLES, 0, 24, skeleton->num_bones());
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	void render_mesh(SkeletalMesh* mesh, const ObjectUniforms& transforms, const PoseTransforms& bones)
	{
		// Bind uniform buffers.
		m_object_ubo->bind_base(1);
		m_bone_ubo->bind_base(2);

		// Bind vertex array.
		mesh->bind_vao();

		for (uint32_t i = 0; i < mesh->num_sub_meshes(); i++)
		{
			SubMesh& submesh = mesh->sub_mesh(i);

			// Issue draw call.
			glDrawElementsBaseVertex(GL_TRIANGLES, submesh.num_indices, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * submesh.base_index), submesh.base_vertex);
		}
	}
    
	// -----------------------------------------------------------------------------------------------------------------------------------

	void render_skeletal_meshes()
	{
		// Bind shader program.
		m_anim_program->use();

		// Bind uniform buffers.
		m_global_ubo->bind_base(0);

		// Draw meshes.
		render_mesh(m_skeletal_mesh.get(), m_character_transforms, m_pose_transforms);
	}
    
	// -----------------------------------------------------------------------------------------------------------------------------------

	void update_object_uniforms(const ObjectUniforms& transform)
	{
        void* ptr = m_object_ubo->map(GL_WRITE_ONLY);
		memcpy(ptr, &transform, sizeof(ObjectUniforms));
        m_object_ubo->unmap();
	}
    
    // -----------------------------------------------------------------------------------------------------------------------------------
    
    void update_global_uniforms(const GlobalUniforms& global)
    {
        void* ptr = m_global_ubo->map(GL_WRITE_ONLY);
        memcpy(ptr, &global, sizeof(GlobalUniforms));
        m_global_ubo->unmap();
    }

	// -----------------------------------------------------------------------------------------------------------------------------------

	void update_bone_uniforms(PoseTransforms* bones)
	{
		void* ptr = m_bone_ubo->map(GL_WRITE_ONLY);
		memcpy(ptr, bones, sizeof(PoseTransforms));
		m_bone_ubo->unmap();
	}
    
    // -----------------------------------------------------------------------------------------------------------------------------------
    
    void update_transforms(dw::Camera* camera)
    {
        // Update camera matrices.
        m_global_uniforms.view = camera->m_view;
        m_global_uniforms.projection = camera->m_projection;
        
        // Update plane transforms.
        m_plane_transforms.model = glm::mat4(1.0f);

        // Update character transforms.
		m_character_transforms.model = glm::rotate(m_plane_transforms.model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        m_character_transforms.model = glm::scale(m_character_transforms.model, glm::vec3(0.1f));
    }

    // -----------------------------------------------------------------------------------------------------------------------------------
    
    void update_camera()
    {
        dw::Camera* current = m_main_camera.get();
        
        if (m_debug_mode)
            current = m_debug_camera.get();
        
        float forward_delta = m_heading_speed * m_delta;
        float right_delta = m_sideways_speed * m_delta;
        
        current->set_translation_delta(current->m_forward, forward_delta);
        current->set_translation_delta(current->m_right, right_delta);

		double d = 1 - exp(log(0.5) * m_springness * m_delta_seconds);

		m_camera_x = m_mouse_delta_x * m_camera_sensitivity;
		m_camera_y = m_mouse_delta_y * m_camera_sensitivity;
        
        if (m_mouse_look)
        {
            // Activate Mouse Look
            current->set_rotatation_delta(glm::vec3((float)(m_camera_y),
                                                    (float)(m_camera_x),
                                                    (float)(0.0f)));
        }
        else
        {
            current->set_rotatation_delta(glm::vec3((float)(0),
                                                    (float)(0),
                                                    (float)(0)));
        }
        
        current->update();

		update_transforms(current);
    }
    
    // -----------------------------------------------------------------------------------------------------------------------------------
    
    void gui()
    {
		visualize_hierarchy(m_skeletal_mesh->skeleton());
    }

	// -----------------------------------------------------------------------------------------------------------------------------------

	void update_animations()
	{
		//// Sample
		//Pose* walk_pose = m_walk_sampler->sample(m_delta_seconds);
		//Pose* run_pose = m_run_sampler->sample(m_delta_seconds);
		Pose* additive_pose = m_additive_sampler->sample(m_delta_seconds);

		//// Blend
		//Pose* blend_pose = m_blend->blend(walk_pose, run_pose, m_blend_factor);
		//Pose* final_pose = m_blend->blend_partial_additive(blend_pose, additive_pose, m_additive_blend_factor, "Spine3");
		
		Pose* locomotion_pose = m_blendspace_1d->evaluate(m_delta_seconds);
		Pose* final_pose = m_blend->blend_partial_additive_with_reference(locomotion_pose, additive_pose, m_additive_blend_factor, "spine_02");

		PoseTransforms* transforms = m_bone_to_local->generate_transforms(final_pose);

		update_bone_uniforms(transforms);
		update_skeleton_debug(m_skeletal_mesh->skeleton(), m_bone_to_local->global_transforms());
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	void update_skeleton_debug(Skeleton* skeleton, PoseTransforms* transforms)
	{
		m_joint_pos.clear();

		Joint* joints = skeleton->joints();

		for (int i = 0; i < skeleton->num_bones(); i++)
		{
			glm::mat4 joint = joints[i].offset_transform;
			glm::mat4 mat = m_character_transforms.model * transforms->transforms[i];

			glm::vec4 p = mat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			m_joint_pos.push_back(glm::vec3(p.x, p.y, p.z));
		}
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	void visualize_skeleton(Skeleton* skeleton)
	{
		for (int i = 0; i < m_joint_pos.size(); i++)
		{
			glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);

			if (i == 0)
				color = glm::vec3(0.0f, 0.0f, 1.0f);
	
			if (m_selected_node == i)
				color = glm::vec3(0.0f, 1.0f, 0.0f);

			m_debug_draw.sphere(0.1f, m_joint_pos[i], color);
		}
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	void visualize_bones(Skeleton* skeleton)
	{
		Joint* joints = skeleton->joints();

		for (int i = 0; i < skeleton->num_bones(); i++)
		{
			if (joints[i].parent_index == -1)
				continue;

			m_debug_draw.line(m_joint_pos[i], m_joint_pos[joints[i].parent_index], glm::vec3(0.0f, 1.0f, 0.0f));
		}
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	void visualize_hierarchy(Skeleton* skeleton)
	{
		static bool skeleton_window = true;

		ImGui::Begin("Skeletal Animation", &skeleton_window);

		ImGui::Checkbox("Visualize Mesh", &m_visualize_mesh);
		ImGui::Checkbox("Visualize Joints", &m_visualize_joints);
		ImGui::Checkbox("Visualize Bones", &m_visualize_bones);

		float rate = m_walk_sampler->playback_rate();
		ImGui::SliderFloat("Playback Rate", &rate, 0.1f, 1.0f);
		m_walk_sampler->set_playback_rate(rate);

		float min = m_blendspace_1d->min();
		float max = m_blendspace_1d->max();
		float value = m_blendspace_1d->value();
		ImGui::SliderFloat("Speed", &value, min, max);
		m_blendspace_1d->set_value(value);

		ImGui::SliderFloat("Additive Weight", &m_additive_blend_factor, 0.0f, 1.0f);

		ImGui::Separator();

		ImGui::Text("Hierarchy");

		Joint* joints = skeleton->joints();

		for (int i = 0; i < skeleton->num_bones(); i++)
		{
			if (m_index_stack.size() > 0 && joints[i].parent_index < m_index_stack.back().first)
			{
				while (m_index_stack.back().first != joints[i].parent_index)
				{
					if (m_index_stack.back().second)
						ImGui::TreePop();

					m_index_stack.pop_back();
				}
			}
			
			bool parent_opened = false;

			for (auto& p : m_index_stack)
			{
				if (p.first == joints[i].parent_index && p.second)
				{
					parent_opened = true;
					break;
				}
			}

			if (!parent_opened && m_index_stack.size() > 0)
				continue;

			ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (m_selected_node == i ? ImGuiTreeNodeFlags_Selected : 0);
			bool opened = ImGui::TreeNodeEx(joints[i].name.c_str(), node_flags);

			if (ImGui::IsItemClicked())
				m_selected_node = i;
	
			m_index_stack.push_back({ i, opened });
		}

		if (m_index_stack.size() > 0)
		{
			while (m_index_stack.size() > 0)
			{
				if (m_index_stack.back().second)
					ImGui::TreePop();

				m_index_stack.pop_back();
			}
		}

		ImGui::End();
	}
    
    // -----------------------------------------------------------------------------------------------------------------------------------
    
private:
	// General GPU resources.
    std::unique_ptr<dw::Shader> m_vs;
	std::unique_ptr<dw::Shader> m_fs;
	std::unique_ptr<dw::Program> m_program;
	std::unique_ptr<dw::UniformBuffer> m_object_ubo;
    std::unique_ptr<dw::UniformBuffer> m_bone_ubo;
    std::unique_ptr<dw::UniformBuffer> m_global_ubo;
    
    // Animation shaders.
    std::unique_ptr<dw::Shader> m_anim_vs;
    std::unique_ptr<dw::Shader> m_anim_fs;
    std::unique_ptr<dw::Program> m_anim_program;

	// Bone shaders.
	std::unique_ptr<dw::Shader> m_bone_vs;
	std::unique_ptr<dw::Shader> m_bone_fs;
	std::unique_ptr<dw::Program> m_bone_program;

    // Camera.
    std::unique_ptr<dw::Camera> m_main_camera;
    std::unique_ptr<dw::Camera> m_debug_camera;
    
	// Uniforms.
	ObjectUniforms m_plane_transforms;
    ObjectUniforms m_character_transforms;
    GlobalUniforms m_global_uniforms;
	PoseTransforms m_pose_transforms;

	// Animations
	std::unique_ptr<Animation> m_walk_animation;
	std::unique_ptr<Animation> m_jog_animation;
	std::unique_ptr<Animation> m_run_animation;
	std::unique_ptr<Animation> m_additive_animation;
	std::unique_ptr<AnimSample> m_walk_sampler;
	std::unique_ptr<AnimSample> m_run_sampler;
	std::unique_ptr<AnimSample> m_additive_sampler;
	std::unique_ptr<AnimBoneToLocal> m_bone_to_local;
	std::unique_ptr<AnimBlend> m_blend;
	std::unique_ptr<Blendspace1D> m_blendspace_1d;

	// Mesh
	std::unique_ptr<SkeletalMesh> m_skeletal_mesh;

	// Bone Mesh
	std::unique_ptr<dw::VertexBuffer> m_bone_vbo;
	std::unique_ptr<dw::IndexBuffer>  m_bone_ibo;
	std::unique_ptr<dw::VertexArray>  m_bone_vao;

    // Camera controls.
    bool m_mouse_look = false;
    bool m_debug_mode = false;
    float m_heading_speed = 0.0f;
    float m_sideways_speed = 0.0f;
    float m_camera_sensitivity = 0.05f;
    float m_camera_speed = 0.01f;

	// GUI
	bool m_visualize_mesh = true;
	bool m_visualize_joints = false;
	bool m_visualize_bones = false;

	// Camera orientation.
	float m_camera_x;
	float m_camera_y;
	float m_springness = 1.0f;
	float m_blend_factor = 0.0f;
	float m_additive_blend_factor = 0.0f;

	int32_t m_selected_node = -1;
	std::vector<glm::vec3> m_joint_pos;
	std::vector<std::pair<int32_t, bool>> m_index_stack;
};

DW_DECLARE_MAIN(AnimationStateMachine)
