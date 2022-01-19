#ifndef RENDERER_HPP
  #define RENDERER_HPP

#include <vector>

#include <libretro_vulkan.h>

#include "camera.hpp"
#include "graphics/vertex.hpp"
#include <transform.hpp>

#define MAX_SYNC 4
#define WIDTH 1280
#define HEIGHT 720

namespace volcano {
	struct buffer {
		VkBuffer buffer;
		VkDeviceMemory memory;
	};

	class mesh;

	class renderer {
		private:
			VkPhysicalDeviceProperties gpu_properties;
			VkPhysicalDeviceMemoryProperties memory_properties;

			unsigned num_swapchain_images;
			uint32_t swapchain_mask;
			VkDeviceMemory image_memory[MAX_SYNC];
			struct retro_vulkan_image images[MAX_SYNC];
			VkFramebuffer framebuffers[MAX_SYNC];

			VkPipelineCache pipeline_cache;
			VkPipelineLayout pipeline_layout;
			VkRenderPass render_pass;
			VkPipeline pipeline;

			VkCommandPool cmd_pool[MAX_SYNC];
			VkCommandBuffer cmd[MAX_SYNC];

			VkDescriptorPool desc_pool;
			VkDescriptorSetLayout set_layout;
			VkDescriptorSet desc_set[MAX_SYNC];
			VkDescriptorSetLayout set_layout_model;

			struct buffer ubo[MAX_SYNC];

			unsigned int index;

			float cam_x, cam_y;
			camera main_camera;

			std::vector<mesh> meshes;

			uint32_t find_memory_type_from_requirements(uint32_t device_requirements, uint32_t host_requirements);
			struct buffer create_buffer(const void *initial, size_t size, VkBufferUsageFlags usage);
			void init_uniform_buffer();
			void init_command();
			void init_descriptor();
			void init_render_pass(VkFormat format);
			VkShaderModule create_shader_module(const uint32_t *data, size_t size);
			void init_pipeline();
			void init_swapchain();
			void update_ubo();

		public:
			renderer();

			void init(retro_hw_render_interface_vulkan *);
			void dispatch();

			void add_mesh(
				const std::vector<graphics::vertex>& vertices,
				const kepler::transform_reference& transform
			);

			void move_forward();
			void move_backward();
			void move_left();
			void move_right();

			const camera& get_camera();
	};
}

#endif
