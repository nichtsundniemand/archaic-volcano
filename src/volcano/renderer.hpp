#ifndef RENDERER_HPP
  #define RENDERER_HPP

#include <libretro_vulkan.h>

#define MAX_SYNC 4

namespace volcano {
	struct buffer {
		VkBuffer buffer;
		VkDeviceMemory memory;
	};

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

			struct buffer ubo[MAX_SYNC];

			unsigned int index;

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
			void init(retro_hw_render_interface_vulkan *);
			void dispatch();
	};
}

#endif
