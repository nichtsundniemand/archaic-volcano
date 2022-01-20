#include <libretro_vulkan.h>

#include <vector>

#include <loguru.hpp>

#include "volcano/graphics/vertex.hpp"
#include "volcano/mesh.hpp"
#include "volcano/renderer.hpp"

#define WIDTH 1280
#define HEIGHT 720

static const char *library_name     = "Archaic Volcano";
static const char *library_version  = "v0.0.1";
static const char *valid_extensions = "";

static struct retro_callbacks {
	retro_environment_t env;
	retro_video_refresh_t video;
	retro_audio_sample_t audio;
	retro_audio_sample_batch_t audio_batch;
	retro_input_poll_t input_poll;
	retro_input_state_t input_state;
} retro_callbacks;

static retro_hw_render_interface_vulkan *vulkan;
volcano::renderer renderer;

// Core basics
RETRO_API unsigned int retro_api_version() {
	return RETRO_API_VERSION;
}

RETRO_API void retro_get_system_info(retro_system_info *info) {
	info->library_name     = library_name;
	info->library_version  = library_version;
	info->valid_extensions = valid_extensions;
	info->need_fullpath    = false;
	info->block_extract    = false;
}

RETRO_API void retro_set_environment(retro_environment_t cb) {
	retro_callbacks.env = cb;

	bool no_rom = true;
	cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_rom);
}

RETRO_API void retro_init() {
	loguru::add_file("logs/volcano_debug.log", loguru::Append, loguru::Verbosity_INFO);
	loguru::g_stderr_verbosity = loguru::Verbosity_ERROR;
	LOG_F(INFO, "Logger initialized!");
}

RETRO_API void retro_deinit() {}

RETRO_API void retro_set_video_refresh(retro_video_refresh_t cb) {
	retro_callbacks.video = cb;
}

RETRO_API void retro_set_audio_sample(retro_audio_sample_t cb) {
	retro_callbacks.audio = cb;
}

RETRO_API void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) {
	retro_callbacks.audio_batch = cb;
}

RETRO_API void retro_set_input_poll(retro_input_poll_t cb) {
	retro_callbacks.input_poll = cb;
}

RETRO_API void retro_set_input_state(retro_input_state_t cb) {
	retro_callbacks.input_state = cb;
}

RETRO_API void retro_set_controller_port_device(
	[[maybe_unused]] unsigned port,
	[[maybe_unused]] unsigned device
) {}

// Core runtime stuff
RETRO_CALLCONV void retro_context_reset() {
	if(!retro_callbacks.env(RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE, (void **)&vulkan) || !vulkan) {
		LOG_F(FATAL, "Could not fetch HW-render interface from frontend!");
		return;
	}

	LOG_F(
		INFO, "Successfully fetched HW-interface: vulkan:\n\tinterface_version: %d\n\thandle: %p\n",
		vulkan->interface_version, vulkan->handle
	);

	renderer.init(vulkan);

	// Create a simple colored triangle
	// static const float data[] = {
	//   -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // vec4 position, vec4 color
	//   -0.5f, +0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
	//   +0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	// };
	// renderer.add_mesh(data, sizeof(data) / sizeof(float));

	// Some other mesh
	const float circle_radius = 0.75f;
	const int circle_segments = 64;
	std::vector<volcano::graphics::vertex> circle_data;
	for(int i = 0; i < circle_segments; i++) {
		// Center vertex
		glm::vec3 p(0.0f, 0.0f, 0.0f);
		circle_data.push_back({
			.position = {0.0f, 0.0f, 0.0f},
			.normal   = {0.0f, 0.0f, 0.0f},
			.color    = {0.5f, 0.0f, 0.0f, 1.0f}
		});

		// Diameter vertices (first)
		circle_data.push_back({
			.position = {
				sin(i * 2 * M_PI / circle_segments) * circle_radius,
				cos(i * 2 * M_PI / circle_segments) * circle_radius,
				0.03f * (i - (circle_segments >> 1))
			},
			.normal = {0.0f, 0.0f, 0.0f},
			.color  = {0.5f, 0.0f, 0.1f * i, 1.0f}
		});

		// Position data (second)
		circle_data.push_back({
			.position = {
				sin((i + 1) * 2 * M_PI / circle_segments) * circle_radius,
				cos((i + 1) * 2 * M_PI / circle_segments) * circle_radius,
				0.03f * (i + 1 - (circle_segments >> 1)),
			},
			.normal = {0.0f, 0.0f, 0.0f},
			.color  = {0.5f, 0.0f, 0.1f * (i + 1), 1.0f}
		});
	}
	// renderer.add_mesh(circle_data);

	const float cube_length = 0.75f;
	std::vector<volcano::graphics::vertex> cube_data;
	// Back face
	glm::vec3 back_normal(0.0f, 0.0f, -1.0f);
	glm::vec4 back_color(0.5f, 1.0f, 1.0f, 1.0f);
	cube_data.push_back({
		.position = {-cube_length / 2, cube_length / 2, -cube_length / 2},
		.normal = back_normal,
		.color = back_color
	});
	cube_data.push_back({
		.position = {-cube_length / 2, -cube_length / 2, -cube_length / 2},
		.normal = back_normal,
		.color = back_color
	});
	cube_data.push_back({
		.position = {cube_length / 2, -cube_length / 2, -cube_length / 2},
		.normal = back_normal,
		.color = back_color
	});
	cube_data.push_back({
		.position = {cube_length / 2, -cube_length / 2, -cube_length / 2},
		.normal = back_normal,
		.color = back_color
	});
	cube_data.push_back({
		.position = {cube_length / 2, cube_length / 2, -cube_length / 2},
		.normal = back_normal,
		.color = back_color
	});
	cube_data.push_back({
		.position = {-cube_length / 2, cube_length / 2, -cube_length / 2},
		.normal = back_normal,
		.color = back_color
	});
	// Front face
	glm::vec3 front_normal(0.0f, 0.0f, 1.0f);
	glm::vec4 front_color(1.0f, 0.5f, 1.0f, 1.0f);
	cube_data.push_back({
		.position = {-cube_length / 2, cube_length / 2, cube_length / 2},
		.normal = front_normal,
		.color = front_color
	});
	cube_data.push_back({
		.position = {cube_length / 2, -cube_length / 2, cube_length / 2},
		.normal = front_normal,
		.color = front_color
	});
	cube_data.push_back({
		.position = {-cube_length / 2, -cube_length / 2, cube_length / 2},
		.normal = front_normal,
		.color = front_color
	});
	cube_data.push_back({
		.position = {cube_length / 2, -cube_length / 2, cube_length / 2},
		.normal = front_normal,
		.color = front_color
	});
	cube_data.push_back({
		.position = {-cube_length / 2, cube_length / 2, cube_length / 2},
		.normal = front_normal,
		.color = front_color
	});
	cube_data.push_back({
		.position = {cube_length / 2, cube_length / 2, cube_length / 2},
		.normal = front_normal,
		.color = front_color
	});
	// Left face
	glm::vec3 left_normal(-1.0f, 0.0f, 0.0f);
	glm::vec4 left_color(1.0f, 1.0f, 0.5f, 1.0f);
	cube_data.push_back({
		.position = {-cube_length / 2, cube_length / 2, -cube_length / 2},
		.normal = left_normal,
		.color = left_color
	});
	cube_data.push_back({
		.position = {-cube_length / 2, -cube_length / 2, cube_length / 2},
		.normal = left_normal,
		.color = left_color
	});
	cube_data.push_back({
		.position = {-cube_length / 2, -cube_length / 2, -cube_length / 2},
		.normal = left_normal,
		.color = left_color
	});
	cube_data.push_back({
		.position = {-cube_length / 2, -cube_length / 2, cube_length / 2},
		.normal = left_normal,
		.color = left_color
	});
	cube_data.push_back({
		.position = {-cube_length / 2, cube_length / 2, -cube_length / 2},
		.normal = left_normal,
		.color = left_color
	});
	cube_data.push_back({
		.position = {-cube_length / 2, cube_length / 2, cube_length / 2},
		.normal = left_normal,
		.color = left_color
	});
	// Right face
	glm::vec3 right_normal(1.0f, 0.0f, 0.0f);
	glm::vec4 right_color(0.5f, 1.0f, 0.5f, 1.0f);
	cube_data.push_back({
		.position = {cube_length / 2, cube_length / 2, -cube_length / 2},
		.normal = right_normal,
		.color = right_color
	});
	cube_data.push_back({
		.position = {cube_length / 2, -cube_length / 2, -cube_length / 2},
		.normal = right_normal,
		.color = right_color
	});
	cube_data.push_back({
		.position = {cube_length / 2, -cube_length / 2, cube_length / 2},
		.normal = right_normal,
		.color = right_color
	});
	cube_data.push_back({
		.position = {cube_length / 2, -cube_length / 2, cube_length / 2},
		.normal = right_normal,
		.color = right_color
	});
	cube_data.push_back({
		.position = {cube_length / 2, cube_length / 2, cube_length / 2},
		.normal = right_normal,
		.color = right_color
	});
	cube_data.push_back({
		.position = {cube_length / 2, cube_length / 2, -cube_length / 2},
		.normal = right_normal,
		.color = right_color
	});
	// Top face
	glm::vec3 top_normal(0.0f, 1.0f, 0.0f);
	glm::vec4 top_color(1.0f, 0.5f, 0.5f, 1.0f);
	cube_data.push_back({
		.position = {-cube_length / 2, cube_length / 2, -cube_length / 2},
		.normal = top_normal,
		.color = top_color
	});
	cube_data.push_back({
		.position = {cube_length / 2, cube_length / 2, cube_length / 2},
		.normal = top_normal,
		.color = top_color
	});
	cube_data.push_back({
		.position = {-cube_length / 2, cube_length / 2, cube_length / 2},
		.normal = top_normal,
		.color = top_color
	});
	cube_data.push_back({
		.position = {cube_length / 2, cube_length / 2, cube_length / 2},
		.normal = top_normal,
		.color = top_color
	});
	cube_data.push_back({
		.position = {-cube_length / 2, cube_length / 2, -cube_length / 2},
		.normal = top_normal,
		.color = top_color
	});
	cube_data.push_back({
		.position = {cube_length / 2, cube_length / 2, -cube_length / 2},
		.normal = top_normal,
		.color = top_color
	});
	// Bottom face
	glm::vec3 bottom_normal(0.0f, -1.0f, 0.0f);
	glm::vec4 bottom_color(0.5f, 0.5f, 1.0f, 1.0f);
	cube_data.push_back({
		.position = {-cube_length / 2, -cube_length / 2, -cube_length / 2},
		.normal = bottom_normal,
		.color = bottom_color
	});
	cube_data.push_back({
		.position = {-cube_length / 2, -cube_length / 2, cube_length / 2},
		.normal = bottom_normal,
		.color = bottom_color
	});
	cube_data.push_back({
		.position = {cube_length / 2, -cube_length / 2, cube_length / 2},
		.normal = bottom_normal,
		.color = bottom_color
	});
	cube_data.push_back({
		.position = {cube_length / 2, -cube_length / 2, cube_length / 2},
		.normal = bottom_normal,
		.color = bottom_color
	});
	cube_data.push_back({
		.position = {cube_length / 2, -cube_length / 2, -cube_length / 2},
		.normal = bottom_normal,
		.color = bottom_color
	});
	cube_data.push_back({
		.position = {-cube_length / 2, -cube_length / 2, -cube_length / 2},
		.normal = bottom_normal,
		.color = bottom_color
	});
	renderer.add_mesh(cube_data);
}

RETRO_CALLCONV void retro_context_destroy() {
	LOG_F(INFO, "Wow - really should've been doing that, huh?");
}

RETRO_API bool retro_load_game([[maybe_unused]] const struct retro_game_info *game) {
	// Initialize vulkan-context
	static struct retro_hw_render_callback hw_render = {
		.context_type    = RETRO_HW_CONTEXT_VULKAN,
		.context_reset   = &retro_context_reset,
		.version_major   = VK_MAKE_VERSION(1, 0, 18),
		.version_minor   = 0,
		.cache_context   = true,
		.context_destroy = &retro_context_destroy,
	};
	if(!retro_callbacks.env(RETRO_ENVIRONMENT_SET_HW_RENDER, &hw_render))
		return false;

	static const struct retro_hw_render_context_negotiation_interface_vulkan iface = {
		RETRO_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_VULKAN,
		RETRO_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_VULKAN_VERSION,

		// get_application_info,
		NULL,
		NULL,
	};

	retro_callbacks.env(RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE, (void *)&iface);

	return true;
}

RETRO_API bool retro_load_game_special(
	[[maybe_unused]] unsigned game_type,
	[[maybe_unused]] const struct retro_game_info *info,
	[[maybe_unused]] size_t num_info
) {
	return false;
}

RETRO_API void retro_get_system_av_info(retro_system_av_info *info) {
	info->geometry = retro_game_geometry {
		.base_width  = WIDTH,
		.base_height = HEIGHT,
		.max_width   = 1920,
		.max_height  = 1080,
	};
	info->timing = retro_system_timing {
		.fps         = 60.0,
		.sample_rate = 48000.0,
	};
}

RETRO_API unsigned retro_get_region(void) {
	return RETRO_REGION_PAL;
}

RETRO_API void retro_run(void) {
	renderer.dispatch();

	retro_callbacks.video(RETRO_HW_FRAME_BUFFER_VALID, WIDTH, HEIGHT, 0);
}

RETRO_API void retro_reset(void) {}

RETRO_API void retro_unload_game(void) {}

// Memory extraction
RETRO_API size_t retro_get_memory_size([[maybe_unused]] unsigned id) {
	return 0;
}

RETRO_API void *retro_get_memory_data([[maybe_unused]] unsigned id) {
	return nullptr;
}

// Serialization
RETRO_API size_t retro_serialize_size(void) {
	return 0;
}

RETRO_API bool retro_serialize(
	[[maybe_unused]] void *data,
	[[maybe_unused]] size_t size
) {
	return false;
}

RETRO_API bool retro_unserialize(
	[[maybe_unused]] const void *data,
	[[maybe_unused]] size_t size
) {
	return false;
}

// Cheat related
RETRO_API void retro_cheat_reset(void) {}

RETRO_API void retro_cheat_set(
	[[maybe_unused]] unsigned index,
	[[maybe_unused]] bool enabled,
	[[maybe_unused]] const char *code
) {}
