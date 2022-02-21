#include <libretro_vulkan.h>

#include <vector>

#include <loguru.hpp>

#include <kepler/transform.hpp>
#include "kraken/dispatcher.hpp"
#include "volcano/graphics/meshes.hpp"
#include "volcano/graphics/vertex.hpp"
#include "volcano/mesh.hpp"
#include "volcano/mesh_loader.hpp"
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

kraken::dispatcher dispatcher;

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

static float cam_x = 0;
static float cam_y = 0;

// Core runtime stuff
kepler::transform grid_transform;
kepler::transform cube_transform;
kepler::transform table_transform, chair_transform;

std::array<kepler::transform, 1024> random_transforms;
unsigned int random_transform_count = 0;

glm::vec3 cam_offset(0, 5, 9);

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

	renderer.get_camera().set_eye(cam_offset);
	renderer.get_camera().set_target(glm::vec3(0, 0, 0));

	auto grid = volcano::graphics::make_grid(43, 43);
	renderer.add_mesh(grid, grid_transform);

	auto cube = volcano::graphics::make_cube();
	renderer.add_mesh(cube, cube_transform);

	volcano::mesh_loader table_loader(
		"data/meshes/test_house.bin",
		{
			.buffer_size  = 11460,
			.vertex_start = 0,
			.vertex_count = 390,
			.normal_start = 5680,
			.normal_count = 390,
			.index_start  = 9360,
			.index_count  = 1050,
		}
	);
	table_transform.set_position(glm::vec3(0, 0, -3));
	table_transform.set_orientation(glm::quat(glm::vec3(0, glm::radians(-90.f), 0)));
	renderer.add_mesh(table_loader.vertices(), table_transform);

	volcano::mesh_loader chair_loader(
		"data/meshes/test_chair.bin",
		{
			.buffer_size  = 4632,
			.vertex_start = 0,
			.vertex_count = 170,
			.normal_start = 2040,
			.normal_count = 170,
			.index_start  = 4080,
			.index_count  = 276,
		}
	);
	chair_transform.set_position(glm::vec3(4, 0, 0));
	renderer.add_mesh(chair_loader.vertices(), chair_transform);
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
	{
		LOG_SCOPE_F(INFO, "Poll Input");
		retro_callbacks.input_poll();

		static bool b_pressed = false;

		short input_mask = retro_callbacks.input_state(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK);
		{
			LOG_SCOPE_F(INFO, "Input mask: %d", input_mask);
			if(input_mask & (1 << RETRO_DEVICE_ID_JOYPAD_B)) {
				LOG_F(INFO, "RETRO_DEVICE_ID_JOYPAD_B");
				if(!b_pressed && random_transform_count < 1024) {
					// Just drop in a new mesh :P
					kepler::transform& transform = random_transforms[random_transform_count];
					transform.set_position(glm::vec3(cam_x, 0, cam_y));

					auto cube = volcano::graphics::make_cube();
					renderer.add_mesh(cube, transform);

					random_transform_count++;

					b_pressed = true;
				}
			} else if(b_pressed) {
				b_pressed = false;
			}

			float cube_speed = 0.05f;
			bool pos_changed = false;
			glm::vec3 pos_delta(0, 0, 0);
			if(input_mask & (1 << RETRO_DEVICE_ID_JOYPAD_UP)) {
				LOG_F(INFO, "RETRO_DEVICE_ID_JOYPAD_UP");
				pos_delta.z -= cube_speed;

				pos_changed = true;
			}
			if(input_mask & (1 << RETRO_DEVICE_ID_JOYPAD_DOWN)) {
				LOG_F(INFO, "RETRO_DEVICE_ID_JOYPAD_DOWN");
				pos_delta.z += cube_speed;

				pos_changed = true;
			}
			if(input_mask & (1 << RETRO_DEVICE_ID_JOYPAD_LEFT)) {
				LOG_F(INFO, "RETRO_DEVICE_ID_JOYPAD_LEFT");
				pos_delta.x -= cube_speed;

				pos_changed = true;
			}
			if(input_mask & (1 << RETRO_DEVICE_ID_JOYPAD_RIGHT)) {
				LOG_F(INFO, "RETRO_DEVICE_ID_JOYPAD_RIGHT");
				pos_delta.x += cube_speed;

				pos_changed = true;
			}
			if(input_mask & (1 << RETRO_DEVICE_ID_JOYPAD_A)) {
				LOG_F(INFO, "RETRO_DEVICE_ID_JOYPAD_A");
			}

			if(pos_changed) {
				if(glm::length(pos_delta) != 0) {
					pos_delta = glm::normalize(pos_delta) * cube_speed;

					glm::vec3 new_pos = glm::vec3(cam_x, 0, cam_y) + pos_delta;
					cam_x = new_pos.x;
					cam_y = new_pos.z;

					cube_transform.set_position(new_pos);

					renderer.get_camera().set_eye(new_pos + cam_offset);
					renderer.get_camera().set_target(new_pos);
				}
			}
		}
		// dispatcher.do_stuff
	}

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
