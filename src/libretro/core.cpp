#include <libretro.h>

#include <loguru.hpp>

RETRO_API unsigned int retro_api_version() {
	return 0;
}

RETRO_API void retro_set_environment(retro_environment_t) {}

RETRO_API void retro_init() {
	loguru::add_file("logs/volcano_debug.log", loguru::Append, loguru::Verbosity_INFO);
	loguru::g_stderr_verbosity = loguru::Verbosity_ERROR;
	LOG_F(INFO, "Logger initialized!");
}

RETRO_API void retro_deinit() {}

RETRO_API void retro_set_video_refresh(retro_video_refresh_t) {}

RETRO_API void retro_set_audio_sample(retro_audio_sample_t) {}

RETRO_API void retro_set_audio_sample_batch(retro_audio_sample_batch_t) {}

RETRO_API void retro_set_input_poll(retro_input_poll_t) {}

RETRO_API void retro_set_input_state(retro_input_state_t) {}

RETRO_API void retro_set_controller_port_device(
	[[maybe_unused]] unsigned port,
	[[maybe_unused]] unsigned device
) {}

RETRO_API void retro_get_system_info([[maybe_unused]] retro_system_info *info) {}

// Core runtime stuff
RETRO_API bool retro_load_game([[maybe_unused]] const struct retro_game_info *game) {
	return false;
}

RETRO_API bool retro_load_game_special(
	[[maybe_unused]] unsigned game_type,
	[[maybe_unused]] const struct retro_game_info *info,
	[[maybe_unused]] size_t num_info
) {
	return false;
}

RETRO_API void retro_get_system_av_info([[maybe_unused]] retro_system_av_info *info) {}

RETRO_API unsigned retro_get_region(void) {
	return 0;
}

RETRO_API void retro_run(void) {}

RETRO_API void retro_reset(void) {}

RETRO_API void retro_unload_game(void) {}

// Memory extraction
RETRO_API size_t retro_get_memory_size([[maybe_unused]] unsigned id) {
	return 0;
}

RETRO_API void *retro_get_memory_data([[maybe_unused]] unsigned id) {
	return 0;
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
