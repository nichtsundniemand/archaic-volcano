#include <libretro_vulkan.h>

#include <vector>

#include <loguru.hpp>

#include "volcano/graphics/meshes.hpp"
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

	auto grid = volcano::graphics::make_linegrid(7, 5);
	renderer.add_mesh(grid);
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
