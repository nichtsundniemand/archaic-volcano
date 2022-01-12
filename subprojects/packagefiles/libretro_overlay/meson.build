project('libretro')

libretro_include = include_directories('include')
libretro_dep = declare_dependency(
	include_directories: libretro_include
)

meson.override_dependency('libretro', libretro_dep)
