volcano_lib = static_library(
	'volcano',
	'renderer.cpp',
	dependencies: [
		libretro_dep,
		loguru_dep,
	],
)

volcano_dep = declare_dependency(link_with: volcano_lib)
