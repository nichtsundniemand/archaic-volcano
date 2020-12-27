#include <libretro_vulkan.h>

#include <loguru.hpp>

const int w = 1280;
const int h = 720;

static const char* library_name = "Archaic Volcano";
static const char* library_version = "v0.0.1";
static const char* valid_extensions = "";

static struct retro_callbacks {
  retro_video_refresh_t video;
  retro_audio_sample_t audio;
  retro_audio_sample_batch_t audio_batch;
  retro_input_poll_t input_poll;
  retro_input_state_t input_state;
} retro_callbacks;

static unsigned short buffer[w * h];

// Forward declarations
unsigned int abs(int);

// Core basics
RETRO_API unsigned int retro_api_version () {
  return RETRO_API_VERSION;
}

RETRO_API void retro_get_system_info(retro_system_info* info) {
  info->library_name     = library_name;
  info->library_version  = library_version;
  info->valid_extensions = valid_extensions;
  info->need_fullpath    = false;
  info->block_extract    = false;
}

RETRO_API void retro_set_environment(retro_environment_t cb) {
  bool no_rom = true;
  cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_rom);
}

RETRO_API void retro_init() {
  loguru::add_file("logs/volcano_debug.log", loguru::Append, loguru::Verbosity_MAX);
  loguru::g_stderr_verbosity = 1;
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
RETRO_API bool retro_load_game([[maybe_unused]] const struct retro_game_info* game) {
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
    .base_width  = w,
    .base_height = h,
    .max_width   = 1920,
    .max_height  = 1080,
  };
  info->timing = retro_system_timing {
    .fps           = 60.0d,
    .sample_rate   = 48000.0d,
  };
}

RETRO_API unsigned retro_get_region(void) {
  return RETRO_REGION_PAL;
}

RETRO_API void retro_run(void) {
  for(int y = 0; y < h; y++) {
    for(int x = 0; x < w; x++) {
      buffer[x + w * y] = 0;
      if(abs(x - y) > 10) {
        buffer[x + w * y] = 65535;
      }
    }
  }

  retro_callbacks.video(buffer, w, h, sizeof(unsigned short) * w);
}

RETRO_API void retro_reset(void) {}

RETRO_API void retro_unload_game(void) {}

// Memory extraction
RETRO_API size_t retro_get_memory_size([[maybe_unused]] unsigned id) {
  return 0;
}

RETRO_API void* retro_get_memory_data([[maybe_unused]] unsigned id) {
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

// Misc.
unsigned int abs(int x) {
  if(x < 0)
    return -x;

  return x;
}
