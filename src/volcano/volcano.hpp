#ifndef VOLCANO_HPP
  #define VOLCANO_HPP

#include <libretro_vulkan.h>

#define MAX_SYNC 3

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

      VkPipelineCache pipeline_cache;
      VkPipelineLayout pipeline_layout;
      VkRenderPass render_pass;

      VkCommandPool cmd_pool[MAX_SYNC];
      VkCommandBuffer cmd[MAX_SYNC];

      VkDescriptorPool desc_pool;
      VkDescriptorSetLayout set_layout;
      VkDescriptorSet desc_set[MAX_SYNC];

      struct buffer ubo[MAX_SYNC];
      struct buffer vbo;

      uint32_t find_memory_type_from_requirements(uint32_t device_requirements, uint32_t host_requirements);
      struct buffer create_buffer(const void *initial, size_t size, VkBufferUsageFlags usage);
      void init_uniform_buffer();
      void init_vertex_buffer();
      void init_command();
      void init_descriptor();
      void init_render_pass(VkFormat format);

   public:
      void init(retro_hw_render_interface_vulkan*);
  };
}

#endif
