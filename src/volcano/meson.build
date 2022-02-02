libretro_dep = dependency('libretro')
glm_dep = dependency('glm')

volcano_lib = static_library(
	'volcano',
	[
		'graphics/meshes.cpp',
		'mesh.cpp',
		'mesh_loader.cpp',
		'renderer.cpp',
	],
	dependencies: [
		glm_dep,
		kepler_dep,
		libretro_dep,
		loguru_dep,
		shader_dep,
	],
)

volcano_dep = declare_dependency(
	include_directories: src_include,
	link_with: volcano_lib,
)
