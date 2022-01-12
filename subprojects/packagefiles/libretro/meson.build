project('libretro', 'c')

libretro_include = include_directories('include')
vulkan_lib = static_library(
	'vulkan_symbol_wrapper',
	'vulkan/vulkan_symbol_wrapper.c',
	include_directories: libretro_include
)

libretro_dep = declare_dependency(
	link_with: vulkan_lib,
	include_directories: libretro_include
)

meson.override_dependency('libretro', libretro_dep)
