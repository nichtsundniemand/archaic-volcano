#include "renderer.hpp"

#include <vulkan/vulkan_symbol_wrapper.h>

#include <glm/mat4x4.hpp>

#include <cstdio>
#include <cstring>

#include <loguru.hpp>

namespace volcano {
	static retro_hw_render_interface_vulkan *vulkan_if;

	uint32_t renderer::find_memory_type_from_requirements(uint32_t device_requirements, uint32_t host_requirements) {
		const VkPhysicalDeviceMemoryProperties *props = &this->memory_properties;
		for(uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
			if(device_requirements & (1u << i)) {
				if((props->memoryTypes[i].propertyFlags & host_requirements) == host_requirements) {
					return i;
				}
			}
		}

		return 0;
	}

	struct buffer renderer::create_buffer(const void *initial, size_t size, VkBufferUsageFlags usage) {
		struct buffer buffer;
		VkDevice device = vulkan_if->device;

		VkBufferCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size  = size,
			.usage = usage,
		};
		vkCreateBuffer(device, &info, nullptr, &buffer.buffer);

		VkMemoryRequirements mem_reqs;
		vkGetBufferMemoryRequirements(device, buffer.buffer, &mem_reqs);

		VkMemoryAllocateInfo alloc = {
			.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize  = mem_reqs.size,
			.memoryTypeIndex = this->find_memory_type_from_requirements(
				mem_reqs.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			),
		};

		vkAllocateMemory(device, &alloc, nullptr, &buffer.memory);
		vkBindBufferMemory(device, buffer.buffer, buffer.memory, 0);

		if(initial) {
			void *ptr;
			vkMapMemory(device, buffer.memory, 0, size, 0, &ptr);
			memcpy(ptr, initial, size);
			vkUnmapMemory(device, buffer.memory);
		}

		return buffer;
	}

	void renderer::init_uniform_buffer() {
		for(unsigned i = 0; i < this->num_swapchain_images; i++) {
			this->ubo[i] = create_buffer(
				nullptr, sizeof(glm::mat4),
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
			);
		}
	}

	void renderer::init(retro_hw_render_interface_vulkan *vulkan) {
		vulkan_if = vulkan;
		fprintf(stderr, "volcano_init(): Initialization begun!\n");

		vulkan_symbol_wrapper_init(vulkan->get_instance_proc_addr);
		vulkan_symbol_wrapper_load_core_instance_symbols(vulkan->instance);
		vulkan_symbol_wrapper_load_core_device_symbols(vulkan->device);

		vkGetPhysicalDeviceProperties(vulkan->gpu, &this->gpu_properties);
		vkGetPhysicalDeviceMemoryProperties(vulkan->gpu, &this->memory_properties);

		unsigned num_images  = 0;
		this->swapchain_mask = vulkan->get_sync_index_mask(vulkan->handle);
		for(unsigned i = 0; i < 32; i++)
			if(this->swapchain_mask & (1u << i))
				num_images = i + 1;
		this->num_swapchain_images = num_images;

		init_uniform_buffer();

		VkPipelineCacheCreateInfo pipeline_cache_info = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
		vkCreatePipelineCache(vulkan->device, &pipeline_cache_info, nullptr, &this->pipeline_cache);
	}
}
