#include "volcano.hpp"

#include <vulkan/vulkan_symbol_wrapper.h>

#include <cstdio>
#include <cstring>

namespace volcano {
  static retro_hw_render_interface_vulkan* vulkan_if;

  uint32_t renderer::find_memory_type_from_requirements(uint32_t device_requirements, uint32_t host_requirements) {
    const VkPhysicalDeviceMemoryProperties *props = &this->memory_properties;
    for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
      if (device_requirements & (1u << i)) {
        if ((props->memoryTypes[i].propertyFlags & host_requirements) == host_requirements) {
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
      .size = size,
      .usage = usage,
    };
    vkCreateBuffer(device, &info, nullptr, &buffer.buffer);

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(device, buffer.buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = mem_reqs.size,
      .memoryTypeIndex = this->find_memory_type_from_requirements(mem_reqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
      ),
    };

    vkAllocateMemory(device, &alloc, nullptr, &buffer.memory);
    vkBindBufferMemory(device, buffer.buffer, buffer.memory, 0);

    if (initial) {
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
        nullptr, 16 * sizeof(float),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
      );
    }
  }

  void renderer::init_vertex_buffer() {
    // Create a simple colored triangle
    static const float data[] = {
      -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // vec4 position, vec4 color
      -0.5f, +0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
      +0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    };
    this->vbo = create_buffer(data, sizeof(data), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  }

  void renderer::init_command() {
    VkCommandPoolCreateInfo pool_info = {
      .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = vulkan_if->queue_index,
    };

    for (unsigned i = 0; i < this->num_swapchain_images; i++) {
      vkCreateCommandPool(vulkan_if->device, &pool_info, nullptr, &this->cmd_pool[i]);

      VkCommandBufferAllocateInfo info = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = this->cmd_pool[i],
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
      };
      vkAllocateCommandBuffers(vulkan_if->device, &info, &this->cmd[i]);
    }
  }

  void renderer::init_descriptor() {
    VkDevice device = vulkan_if->device;

    // Initialize descriptor pool
    const VkDescriptorPoolSize pool_sizes[1] = {
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, this->num_swapchain_images },
    };

    VkDescriptorPoolCreateInfo pool_info = {
      .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
      .maxSets       = this->num_swapchain_images,
      .poolSizeCount = 1,
      .pPoolSizes    = pool_sizes,
    };
    vkCreateDescriptorPool(device, &pool_info, nullptr, &this->desc_pool);

    // Define descriptor set
    VkDescriptorSetLayoutBinding binding = {
      .binding            = 0,
      .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount    = 1,
      .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
      .pImmutableSamplers = nullptr,
    };

    VkDescriptorSetLayoutCreateInfo set_layout_info = {
      .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 1,
      .pBindings    = &binding,
    };
    vkCreateDescriptorSetLayout(device, &set_layout_info, nullptr, &this->set_layout);

    VkDescriptorSetAllocateInfo alloc_info = {
      .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool     = this->desc_pool,
      .descriptorSetCount = 1,
      .pSetLayouts        = &this->set_layout,
    };
    for (unsigned i = 0; i < this->num_swapchain_images; i++) {
      vkAllocateDescriptorSets(device, &alloc_info, &this->desc_set[i]);

      VkDescriptorBufferInfo buffer_info = {
        .buffer = this->ubo[i].buffer,
        .offset = 0,
        .range  = 16 * sizeof(float),
      };

      VkWriteDescriptorSet write = {
        .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet          = this->desc_set[i],
        .dstBinding      = 0,
        .descriptorCount = 1,
        .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo     = &buffer_info,
      };

      vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
    }

    // Create pipeline-layout for descriptor set
    VkPipelineLayoutCreateInfo layout_info = {
      .sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts    = &this->set_layout,
    };
    vkCreatePipelineLayout(device, &layout_info, nullptr, &this->pipeline_layout);
  }

  void renderer::init_render_pass(VkFormat format) {
    VkAttachmentDescription attachment = {
      .format         = format,
      .samples        = VK_SAMPLE_COUNT_1_BIT,
      .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference color_ref = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkSubpassDescription subpass = {
      .pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = 1,
      .pColorAttachments    = &color_ref,
    };

    VkRenderPassCreateInfo rp_info = {
      .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments    = &attachment,
      .subpassCount    = 1,
      .pSubpasses      = &subpass,
    };
    vkCreateRenderPass(vulkan_if->device, &rp_info, nullptr, &this->render_pass);
  }

  void renderer::init(retro_hw_render_interface_vulkan* vulkan) {
    vulkan_if = vulkan;
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

    init_uniform_buffer();
    init_vertex_buffer();
    init_command();
    init_descriptor();

    VkPipelineCacheCreateInfo pipeline_cache_info = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
    vkCreatePipelineCache(vulkan->device, &pipeline_cache_info, nullptr, &this->pipeline_cache);

    init_render_pass(VK_FORMAT_R8G8B8A8_UNORM);
  }
}
