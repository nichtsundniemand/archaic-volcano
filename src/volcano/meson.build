glm_dep = dependency('glm')

volcano_lib = static_library(
	'volcano',
	'volcano.cpp',
	dependencies: [
		glm_dep,
		libretro_dep,
		loguru_dep,
	],
)

volcano_dep = declare_dependency(link_with: volcano_lib)
