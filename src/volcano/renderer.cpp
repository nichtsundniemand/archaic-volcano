#include "renderer.hpp"
#include "mesh.hpp"

#include <vulkan/vulkan_symbol_wrapper.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <cmath>
#include <cstdio>
#include <cstring>

#include <vector>

#include <loguru.hpp>

#define DEPTH_FORMAT VK_FORMAT_D32_SFLOAT

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

		LOG_SCOPE_FUNCTION(INFO);

		VkBufferCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size  = size,
			.usage = usage,
		};
		vkCreateBuffer(device, &info, nullptr, &buffer.buffer);
		LOG_F(INFO, "Created buffer with size %ld!", size);

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
		LOG_F(INFO, "Allocated device memory with size %ld!", mem_reqs.size);

		if(initial) {
			void *ptr;
			vkMapMemory(device, buffer.memory, 0, size, 0, &ptr);
			memcpy(ptr, initial, size);
			vkUnmapMemory(device, buffer.memory);

			LOG_F(INFO, "Copied initial memory-contents into buffer!");
		}

		return buffer;
	}

	void renderer::init_uniform_buffer() {
		for(unsigned i = 0; i < this->num_swapchain_images; i++) {
			this->ubo[i] = create_buffer(
				nullptr, 2 * sizeof(glm::mat4),
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
			);
		}
	}

	void renderer::init_command() {
		VkCommandPoolCreateInfo pool_info = {
			.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = vulkan_if->queue_index,
		};

		for(unsigned i = 0; i < this->num_swapchain_images; i++) {
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
		static const std::vector<VkDescriptorPoolSize> pool_sizes {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024 },
		};

		VkDescriptorPoolCreateInfo pool_info = {
			.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
			.maxSets       = 256,
			.poolSizeCount = (uint32_t)pool_sizes.size(),
			.pPoolSizes    = pool_sizes.data(),
		};
		vkCreateDescriptorPool(device, &pool_info, nullptr, &this->desc_pool);

		// Define per-frame descriptor set
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

		// Define per-model descriptor set
		VkDescriptorSetLayoutBinding binding_model = {
			.binding            = 0,
			.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount    = 1,
			.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = nullptr,
		};

		VkDescriptorSetLayoutCreateInfo set_layout_info_model = {
			.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = 1,
			.pBindings    = &binding_model,
		};
		vkCreateDescriptorSetLayout(device, &set_layout_info_model, nullptr, &this->set_layout_model);

		for(unsigned i = 0; i < this->num_swapchain_images; i++) {
			vkAllocateDescriptorSets(device, &alloc_info, &this->desc_set[i]);

			VkDescriptorBufferInfo buffer_info = {
				.buffer = this->ubo[i].buffer,
				.offset = 0,
				.range  = 2 * sizeof(glm::mat4),
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

		// Create pipeline-layout for descriptor sets
		static const std::vector<VkDescriptorSetLayout> set_layouts {
			this->set_layout, this->set_layout_model,
		};

		VkPipelineLayoutCreateInfo layout_info = {
			.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = (uint32_t)set_layouts.size(),
			.pSetLayouts    = set_layouts.data(),
		};
		vkCreatePipelineLayout(device, &layout_info, nullptr, &this->pipeline_layout);
	}

	void renderer::init_render_pass(VkFormat format) {
		VkAttachmentDescription color_attachment = {
			.format         = format,
			.samples        = VK_SAMPLE_COUNT_1_BIT,
			.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};

		VkAttachmentReference color_ref = {
			.attachment = 0,
			.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		VkAttachmentDescription depth_attachment = {
			.format         = DEPTH_FORMAT,
			.samples        = VK_SAMPLE_COUNT_1_BIT,
			.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};

		VkAttachmentReference depth_ref = {
			.attachment = 1,
			.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		VkSubpassDescription subpass = {
			.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount    = 1,
			.pColorAttachments       = &color_ref,
			.pDepthStencilAttachment = &depth_ref,
		};

		static const std::vector<VkAttachmentDescription> attachments {
			color_attachment,
			depth_attachment
		};

		VkRenderPassCreateInfo rp_info = {
			.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = (uint32_t)attachments.size(),
			.pAttachments    = attachments.data(),
			.subpassCount    = 1,
			.pSubpasses      = &subpass,
		};
		vkCreateRenderPass(vulkan_if->device, &rp_info, nullptr, &this->render_pass);
	}

	VkShaderModule renderer::create_shader_module(const uint32_t *data, size_t size) {
		VkShaderModule module;

		VkShaderModuleCreateInfo module_info = {
			.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = size,
			.pCode    = data,
		};
		vkCreateShaderModule(vulkan_if->device, &module_info, nullptr, &module);

		return module;
	}

	void renderer::init_pipeline() {
		VkDevice device = vulkan_if->device;

		VkPipelineInputAssemblyStateCreateInfo input_assembly = {
			.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		};

		static const std::vector<VkVertexInputAttributeDescription> attributes {
			{
				.location = 0,
				.binding  = 0,
				.format   = VK_FORMAT_R32G32B32_SFLOAT,
				.offset   = offsetof(graphics::vertex, position),
			}, {
				.location = 1,
				.binding  = 0,
				.format   = VK_FORMAT_R32G32B32A32_SFLOAT,
				.offset   = offsetof(graphics::vertex, color),
			}, {
				.location = 2,
				.binding  = 0,
				.format   = VK_FORMAT_R32G32B32_SFLOAT,
				.offset   = offsetof(graphics::vertex, normal),
			},
		};

		VkVertexInputBindingDescription binding = {
			.binding   = 0,
			.stride    = sizeof(graphics::vertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		};

		VkPipelineVertexInputStateCreateInfo vertex_input = {
			.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount   = 1,
			.pVertexBindingDescriptions      = &binding,
			.vertexAttributeDescriptionCount = (uint32_t)attributes.size(),
			.pVertexAttributeDescriptions    = attributes.data(),
		};

		VkPipelineRasterizationStateCreateInfo raster = {
			.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable        = false,
			.rasterizerDiscardEnable = false,
			.polygonMode             = VK_POLYGON_MODE_FILL,
			.cullMode                = VK_CULL_MODE_BACK_BIT,
			.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable         = false,
			.lineWidth               = 1.0f,
		};

		VkPipelineColorBlendAttachmentState blend_attachment = {
			.blendEnable    = false,
			.colorWriteMask = 0xf,
		};

		VkPipelineColorBlendStateCreateInfo blend = {
			.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.attachmentCount = 1,
			.pAttachments    = &blend_attachment,
		};

		VkPipelineViewportStateCreateInfo viewport = {
			.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.scissorCount  = 1,
		};

		VkPipelineDepthStencilStateCreateInfo depth_stencil = {
			.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable       = VK_TRUE,
			.depthWriteEnable      = VK_TRUE,
			.depthCompareOp        = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_TRUE,
			.stencilTestEnable     = VK_TRUE,
		};

		VkPipelineMultisampleStateCreateInfo multisample = {
			.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		};

		static const std::vector<VkDynamicState> dynamics {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
		};

		VkPipelineDynamicStateCreateInfo dynamic = {
			.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = (uint32_t)dynamics.size(),
			.pDynamicStates    = dynamics.data(),
		};

		static const uint32_t triangle_vert[] =
			#include "triangle.vert.inc"
		;

		static const uint32_t triangle_frag[] =
			#include "triangle.frag.inc"
		;

		static const std::vector<VkPipelineShaderStageCreateInfo> shader_stages {
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage  = VK_SHADER_STAGE_VERTEX_BIT,
				.module = create_shader_module(triangle_vert, sizeof(triangle_vert)),
				.pName  = "main",
			}, {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
				.module = create_shader_module(triangle_frag, sizeof(triangle_frag)),
				.pName  = "main",
			},
		};

		VkGraphicsPipelineCreateInfo pipe = {
			.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount          = (uint32_t)shader_stages.size(),
			.pStages             = shader_stages.data(),
			.pVertexInputState   = &vertex_input,
			.pInputAssemblyState = &input_assembly,
			.pViewportState      = &viewport,
			.pRasterizationState = &raster,
			.pMultisampleState   = &multisample,
			.pDepthStencilState  = &depth_stencil,
			.pColorBlendState    = &blend,
			.pDynamicState       = &dynamic,
			.layout              = this->pipeline_layout,
			.renderPass          = this->render_pass,
		};
		vkCreateGraphicsPipelines(vulkan_if->device, this->pipeline_cache, 1, &pipe, nullptr, &this->pipeline);

		for(auto& shader_stage: shader_stages)
			vkDestroyShaderModule(device, shader_stage.module, nullptr);
	}

	void renderer::init_swapchain() {
		VkDevice device = vulkan_if->device;

		// Create depth-buffer image
		VkImage depth_image;
		VkDeviceMemory depth_image_memory;
		VkImageView depth_image_view;

		VkImageCreateInfo depth_image_create_info = {
			.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.flags         = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
			.imageType     = VK_IMAGE_TYPE_2D,
			.format        = DEPTH_FORMAT,
			.extent        = {
				.width  = WIDTH,
				.height = HEIGHT,
				.depth  = 1
			},
			.mipLevels     = 1,
			.arrayLayers   = 1,
			.samples       = VK_SAMPLE_COUNT_1_BIT,
			.tiling        = VK_IMAGE_TILING_OPTIMAL,
			.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};

		vkCreateImage(device, &depth_image_create_info, nullptr, &depth_image);

		VkMemoryRequirements depth_mem_reqs;
		vkGetImageMemoryRequirements(device, depth_image, &depth_mem_reqs);

		VkMemoryAllocateInfo depth_alloc = {
			.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize  = depth_mem_reqs.size,
			.memoryTypeIndex = find_memory_type_from_requirements(
				depth_mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			),
		};
		vkAllocateMemory(device, &depth_alloc, nullptr, &depth_image_memory);
		vkBindImageMemory(device, depth_image, depth_image_memory, 0);

		VkImageViewCreateInfo depth_image_view_create_info = {
			.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image            = depth_image,
			.viewType         = VK_IMAGE_VIEW_TYPE_2D,
			.format           = DEPTH_FORMAT,
			// Don't set .components here because default 0 is equivalent to identity-swizzle
			.subresourceRange = {
				.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT,
				.baseMipLevel   = 0,
				.levelCount     = 1,
				.baseArrayLayer = 0,
				.layerCount     = 1
			}
		};
		vkCreateImageView(device, &depth_image_view_create_info, nullptr, &depth_image_view);

		for(unsigned i = 0; i < this->num_swapchain_images; i++) {
			VkImageCreateInfo image = {
				.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.flags         = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
				.imageType     = VK_IMAGE_TYPE_2D,
				.format        = VK_FORMAT_R8G8B8A8_UNORM,
				.extent        = {
					.width  = WIDTH,
					.height = HEIGHT,
					.depth  = 1,
				},
				.mipLevels     = 1,
				.arrayLayers   = 1,
				.samples       = VK_SAMPLE_COUNT_1_BIT,
				.tiling        = VK_IMAGE_TILING_OPTIMAL,
				.usage         =
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
					VK_IMAGE_USAGE_SAMPLED_BIT |
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			};
			VkImage newImage;
			vkCreateImage(device, &image, nullptr, &newImage);

			VkMemoryRequirements mem_reqs;
			vkGetImageMemoryRequirements(device, newImage, &mem_reqs);

			VkMemoryAllocateInfo alloc = {
				.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.allocationSize  = mem_reqs.size,
				.memoryTypeIndex = find_memory_type_from_requirements(
					mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
				),
			};
			vkAllocateMemory(device, &alloc, nullptr, &this->image_memory[i]);
			vkBindImageMemory(device, newImage, this->image_memory[i], 0);

			this->images[i].create_info = {
				.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image            = newImage,
				.viewType         = VK_IMAGE_VIEW_TYPE_2D,
				.format           = VK_FORMAT_R8G8B8A8_UNORM,
				.components       = {
					.r = VK_COMPONENT_SWIZZLE_R,
					.g = VK_COMPONENT_SWIZZLE_G,
					.b = VK_COMPONENT_SWIZZLE_B,
					.a = VK_COMPONENT_SWIZZLE_A,
				},
				.subresourceRange = {
					.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel   = 0,
					.levelCount     = 1,
					.baseArrayLayer = 0,
					.layerCount     = 1,
				},
			};

			vkCreateImageView(
				device, &this->images[i].create_info,
				nullptr, &this->images[i].image_view
			);
			this->images[i].image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			const std::vector<VkImageView> image_attachments = {
				this->images[i].image_view,
				depth_image_view,
			};

			VkFramebufferCreateInfo fb_info = {
				.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass      = this->render_pass,
				.attachmentCount = (uint32_t)image_attachments.size(),
				.pAttachments    = image_attachments.data(),
				.width           = WIDTH,
				.height          = HEIGHT,
				.layers          = 1,
			};
			vkCreateFramebuffer(device, &fb_info, nullptr, &this->framebuffers[i]);
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
		init_command();
		init_descriptor();

		VkPipelineCacheCreateInfo pipeline_cache_info = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
		vkCreatePipelineCache(vulkan->device, &pipeline_cache_info, nullptr, &this->pipeline_cache);

		init_render_pass(VK_FORMAT_R8G8B8A8_UNORM);
		init_pipeline();
		init_swapchain();
	}

	void renderer::update_ubo(void) {
		static unsigned frame;

		float translate = 20.0f;

		glm::mat4 projection = glm::perspective(-glm::pi<float>() * 0.25f, -1.0f / 1.0f, 0.1f, 100.f);
		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -5.0f, -translate));
		view = glm::rotate(view, -60 * 0.0031416926535f, glm::vec3(-1.0f, 0.0f, 0.0f));
		view = glm::rotate(view, 50 * 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(4.3f));

		glm::mat4 mv  = view * model;
		glm::mat4 mvp = projection * mv;

		const std::vector<glm::mat4> ubo_matrices = {
			mvp,
			mv,
		};

		float *memmap_mvp = nullptr;
		vkMapMemory(
			vulkan_if->device, this->ubo[this->index].memory,
			0, ubo_matrices.size() * sizeof(glm::mat4), 0, (void **)&memmap_mvp
		);
		memcpy(memmap_mvp, ubo_matrices.data(), ubo_matrices.size() * sizeof(glm::mat4));
		vkUnmapMemory(vulkan_if->device, this->ubo[this->index].memory);

		frame++;
	}

	void renderer::dispatch() {
		vulkan_if->wait_sync_index(vulkan_if->handle);
		this->index = vulkan_if->get_sync_index(vulkan_if->handle);

		update_ubo();

		VkCommandBuffer cmd = this->cmd[this->index];

		VkCommandBufferBeginInfo begin_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		};
		vkResetCommandBuffer(cmd, 0);
		vkBeginCommandBuffer(cmd, &begin_info);

		VkImageMemoryBarrier prepare_rendering = {
			.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext               = nullptr,
			.srcAccessMask       = 0,
			.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
			.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image               = this->images[this->index].create_info.image,
			.subresourceRange    = {
			  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			  .levelCount = 1,
			  .layerCount = 1,
			}
		};
		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			false, 
			0, nullptr,
			0, nullptr,
			1, &prepare_rendering
		);

		VkClearValue clear_color = {
			.color = {
				.float32 = {
					0.8f,
					0.6f,
					0.2f,
					1.0f,
				},
			},
		};

		VkClearValue clear_depth = {
			.depthStencil = {
				.depth   = 1.0f,
				.stencil = 0,
			},
		};

		const std::vector<VkClearValue> clear_values = {
			clear_color,
			clear_depth,
		};

		VkRenderPassBeginInfo rp_begin = {
			.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext             = nullptr,
			.renderPass        = this->render_pass,
			.framebuffer       = this->framebuffers[this->index],
			.renderArea = {
				{     0,      0 },
				{ WIDTH, HEIGHT },
			},
			.clearValueCount   = (uint32_t)clear_values.size(),
			.pClearValues      = clear_values.data(),
		};
		vkCmdBeginRenderPass(cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline);
		vkCmdBindDescriptorSets(
			cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
			this->pipeline_layout, 0,
			1, &this->desc_set[this->index], 0, nullptr
		);

		VkViewport vp = {
			.x        = (WIDTH - HEIGHT) >> 1,
			.y        = 0.0f,
			.width    = HEIGHT,
			.height   = HEIGHT,
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};
		vkCmdSetViewport(cmd, 0, 1, &vp);

		VkRect2D scissor = {
			{     0,      0 },
			{ WIDTH, HEIGHT },
		};

		vkCmdSetScissor(cmd, 0, 1, &scissor);

		VkDeviceSize offset = 0;
		for(mesh& cur_mesh: meshes) {
			vkCmdBindVertexBuffers(cmd, 0, 1, cur_mesh.get_buffer(), &offset);
			vkCmdDraw(cmd, cur_mesh.get_size(), 1, 0, 0);
			LOG_F(MAX, "Added draw-call for buffer %p with %d vertices", (void *)cur_mesh.get_buffer(), cur_mesh.get_size());
		}

		vkCmdEndRenderPass(cmd);

		VkImageMemoryBarrier prepare_presentation = {
			.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext               = nullptr,
			.srcAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT,
			.oldLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.newLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image               = this->images[this->index].create_info.image,
			.subresourceRange    = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.levelCount = 1,
				.layerCount = 1,
			}
		};
		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			false,
			0, nullptr,
			0, nullptr,
			1, &prepare_presentation
		);

		vkEndCommandBuffer(cmd);

		vulkan_if->set_image(vulkan_if->handle, &this->images[this->index], 0, nullptr, VK_QUEUE_FAMILY_IGNORED);
		vulkan_if->set_command_buffers(vulkan_if->handle, 1, &this->cmd[this->index]);
	}

	void renderer::add_mesh(
		const std::vector<graphics::vertex>& vertices,
		const kepler::transform_reference& transform
	) {
		LOG_F(MAX, "Add new mesh (size=%ld)", vertices.size());

		// Create the VkDescriptorSet for attaching this UBO to the shader
		VkDescriptorSetAllocateInfo alloc_info_model = {
			.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool     = this->desc_pool,
			.descriptorSetCount = 1,
			.pSetLayouts        = &this->set_layout_model,
		};

		glm::mat4 model_transform = glm::mat4(1.0f);

		// Allocate Per-Model descriptor-set and backing buffers (double buffered)
		std::array<struct buffer, MAX_SYNC> uniform_buffers;
		std::array<VkDescriptorSet, MAX_SYNC> descriptor_sets;
		for(unsigned int i = 0; i < descriptor_sets.size(); i++) {
			// Create the backing-buffer for the model-transform's UBO
			uniform_buffers[i] = create_buffer(
				&model_transform, sizeof(glm::mat4),
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
			);

			// Allocate a new descriptor set using the model set-layout of the current pipeline
			vkAllocateDescriptorSets(vulkan_if->device, &alloc_info_model, &descriptor_sets[i]);

			VkDescriptorBufferInfo buffer_info = {
				.buffer = uniform_buffers[i].buffer,
				.offset = 0,
				.range  = sizeof(glm::mat4),
			};

			VkWriteDescriptorSet write = {
				.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet          = descriptor_sets[i],
				.dstBinding      = 0,
				.descriptorCount = 1,
				.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.pBufferInfo     = &buffer_info,
			};
			vkUpdateDescriptorSets(vulkan_if->device, 1, &write, 0, nullptr);
		}

		auto vbo = create_buffer(
			vertices.data(), vertices.size() * sizeof(graphics::vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
		);
		LOG_F(MAX, "Buffer created (vbo.buffer=%d)", vbo.buffer);

		meshes.push_back(mesh(descriptor_sets, uniform_buffers, vbo, vertices.size(), transform));
	}
}
