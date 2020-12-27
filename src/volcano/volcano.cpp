#include "volcano.hpp"

#include <vulkan/vulkan_symbol_wrapper.h>

#include <cstdio>

namespace volcano {
  void renderer::init(retro_hw_render_interface_vulkan* vulkan) {
    fprintf(stderr, "volcano_init(): Initialization begun!\n");

    vulkan_symbol_wrapper_init(vulkan->get_instance_proc_addr);
    vulkan_symbol_wrapper_load_core_instance_symbols(vulkan->instance);
    vulkan_symbol_wrapper_load_core_device_symbols(vulkan->device);

    vkGetPhysicalDeviceProperties(vulkan->gpu, &this->gpu_properties);
    vkGetPhysicalDeviceMemoryProperties(vulkan->gpu, &this->memory_properties);

    unsigned num_images = 0;
    uint32_t mask = vulkan->get_sync_index_mask(vulkan->handle);
    for (unsigned i = 0; i < 32; i++)
      if (mask & (1u << i))
        num_images = i + 1;
    this->num_swapchain_images = num_images;
    this->swapchain_mask = mask;

    VkPipelineCacheCreateInfo pipeline_cache_info = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
    vkCreatePipelineCache(vulkan->device, &pipeline_cache_info, nullptr, &this->pipeline_cache);
  }
}
