#ifndef RENDERER_HPP
  #define RENDERER_HPP

#include <libretro_vulkan.h>

#define MAX_SYNC 3

namespace volcano {
	class renderer {
		private:
			VkPhysicalDeviceProperties gpu_properties;
			VkPhysicalDeviceMemoryProperties memory_properties;

			unsigned num_swapchain_images;
			uint32_t swapchain_mask;

			VkPipelineCache pipeline_cache;

		public:
			void init(retro_hw_render_interface_vulkan *);
	};
}

#endif
