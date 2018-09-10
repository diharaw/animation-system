#include <application.h>
#include <mesh.h>
#include <camera.h>
#include <material.h>
#include <memory>
#include <iostream>
#include <stack>
#include "skeletal_mesh.h"

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

		if (!create_uniform_buffer())
			return false;

		// Load mesh.
		if (!load_mesh())
			return false;

		// Load animations.
		//if (!load_animations())
		//	return false;

		// Create camera.
		create_camera();

		for (int i = 0; i < MAX_BONES; i++)
			m_pose_transforms.transforms[i] = glm::mat4(1.0f);

		m_index_stack.reserve(256);

		return true;
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	void update(double delta) override
	{
        // Debug GUI
        gui();
        
		// Update camera.
        update_camera();
        
        // Render scene.
		render_skeletal_meshes();
        
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
		if (code == GLFW_MOUSE_BUTTON_LEFT)
			m_mouse_look = true;
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	void mouse_released(int code) override
	{
		// Disable mouse look.
		if (code == GLFW_MOUSE_BUTTON_LEFT)
			m_mouse_look = false;
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

private:
    
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
		m_skeletal_mesh = std::unique_ptr<SkeletalMesh>(SkeletalMesh::load("mesh/UE4/Idle.fbx"));

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
		m_idle_animation = std::unique_ptr<Animation>(Animation::load("mesh/UE4/Idle.fbx", m_skeletal_mesh->skeleton()));

		if (!m_idle_animation)
		{
			DW_LOG_FATAL("Failed to load animation!");
			return false;
		}

		return true;
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	void create_camera()
	{
        m_main_camera = std::make_unique<dw::Camera>(60.0f, 0.1f, CAMERA_FAR_PLANE, float(m_width) / float(m_height), glm::vec3(0.0f, 5.0f, 20.0f), glm::vec3(0.0f, 0.0, -1.0f));
        m_debug_camera = std::make_unique<dw::Camera>(60.0f, 0.1f, CAMERA_FAR_PLANE * 2.0f, float(m_width) / float(m_height), glm::vec3(0.0f, 5.0f, 20.0f), glm::vec3(0.0f, 0.0, -1.0f));
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

    void render_mesh(dw::Mesh* mesh, const ObjectUniforms& transforms, bool use_textures = false)
	{
        // Copy new data into UBO.
        update_object_uniforms(transforms);

		// Bind uniform buffers.
        m_global_ubo->bind_base(0);
        m_object_ubo->bind_base(1);

		// Bind vertex array.
        mesh->mesh_vertex_array()->bind();

		for (uint32_t i = 0; i < mesh->sub_mesh_count(); i++)
		{
			dw::SubMesh& submesh = mesh->sub_meshes()[i];

			// Bind texture.
            if (use_textures)
                submesh.mat->texture(0)->bind(0);

			// Issue draw call.
			glDrawElementsBaseVertex(GL_TRIANGLES, submesh.index_count, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * submesh.base_index), submesh.base_vertex);
		}
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	void render_mesh(SkeletalMesh* mesh, const ObjectUniforms& transforms, const PoseTransforms& bones)
	{
		// Copy new data into UBO.
		update_object_uniforms(transforms);
		update_bone_uniforms(bones);

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
    
    void render_scene()
    {
        // Update global uniforms.
        update_global_uniforms(m_global_uniforms);

        // Bind and set viewport.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, m_width, m_height);
  
        // Clear default framebuffer.
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Bind states.
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        
        // Bind shader program.
        m_program->use();
        
        // Bind uniform buffers.
		m_bone_ubo->bind_base(2);
        
        // Draw meshes.
    }

	// -----------------------------------------------------------------------------------------------------------------------------------

	void render_skeletal_meshes()
	{
		// Update global uniforms.
		update_global_uniforms(m_global_uniforms);

		// Bind and set viewport.
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, m_width, m_height);

		// Clear default framebuffer.
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Bind states.
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_NONE);

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

	void update_bone_uniforms(const PoseTransforms& bones)
	{
		void* ptr = m_bone_ubo->map(GL_WRITE_ONLY);
		memcpy(ptr, &bones, sizeof(PoseTransforms));
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
        
        if (m_mouse_look)
        {
            // Activate Mouse Look
            current->set_rotatation_delta(glm::vec3((float)(m_mouse_delta_y * m_camera_sensitivity * m_delta),
                                                    (float)(m_mouse_delta_x * m_camera_sensitivity * m_delta),
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
		ImGui::ShowDemoWindow();
		visualize_hierarchy(m_skeletal_mesh->skeleton());
		render_skeleton(m_skeletal_mesh->skeleton());
    }

	// -----------------------------------------------------------------------------------------------------------------------------------

	glm::mat4 world_matrix(glm::mat4 joint)
	{
		glm::mat4 joint_matrix;
		joint_matrix[0] = glm::vec4(glm::normalize(glm::vec3(joint[0].x, joint[0].y, joint[0].z)), 0.0f);
		joint_matrix[1] = glm::vec4(glm::normalize(glm::vec3(joint[1].x, joint[1].y, joint[1].z)), 0.0f);
		joint_matrix[2] = glm::vec4(glm::normalize(glm::vec3(joint[2].x, joint[2].y, joint[2].z)), 0.0f);
		joint_matrix[3] = glm::vec4(glm::vec3(joint[3].x, joint[3].y, joint[3].z), 1.0f);

		glm::vec3 bone_dir = glm::vec3(joint[0].w, joint[1].w, joint[2].w);
		float bone_len = glm::length(bone_dir);

		glm::mat4 world_matrix;
		world_matrix[0] = joint_matrix[0] * bone_len;
		world_matrix[1] = joint_matrix[1] * bone_len;
		world_matrix[2] = joint_matrix[2] * bone_len;
		world_matrix[3] = joint_matrix[3];

		return world_matrix;
	}

	void render_skeleton(Skeleton* skeleton)
	{
		Joint* joints = skeleton->joints();

		for (int i = 0; i < skeleton->num_bones(); i++)
		{
			m_pose_transforms.transforms[i] = joints[i].offset_transform;

			glm::mat4 joint = joints[i].offset_transform;
			glm::mat4 mat = m_character_transforms.model * glm::inverse(joint);

			glm::vec4 p = mat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);

			if (joints[i].parent_index == -1)
				color = glm::vec3(0.0f, 1.0f, 0.0f);

			m_debug_draw.sphere(0.1f, glm::vec3(p.x, p.y, p.z), color);
		}
	}

	// -----------------------------------------------------------------------------------------------------------------------------------

	void visualize_hierarchy(Skeleton* skeleton)
	{
		static bool skeleton_window = true;

		ImGui::Begin("Skeleton Hierarchy", &skeleton_window);

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

    // Camera.
    std::unique_ptr<dw::Camera> m_main_camera;
    std::unique_ptr<dw::Camera> m_debug_camera;
    
	// Uniforms.
	ObjectUniforms m_plane_transforms;
    ObjectUniforms m_character_transforms;
    GlobalUniforms m_global_uniforms;
	PoseTransforms m_pose_transforms;

	// Animations
	std::unique_ptr<Animation> m_idle_animation;

	// Mesh
	std::unique_ptr<SkeletalMesh> m_skeletal_mesh;

    // Camera controls.
    bool m_mouse_look = false;
    bool m_debug_mode = false;
    float m_heading_speed = 0.0f;
    float m_sideways_speed = 0.0f;
    float m_camera_sensitivity = 0.05f;
    float m_camera_speed = 0.01f;

	int32_t m_selected_node = -1;
	std::vector<std::pair<int32_t, bool>> m_index_stack;
};

DW_DECLARE_MAIN(AnimationStateMachine)
