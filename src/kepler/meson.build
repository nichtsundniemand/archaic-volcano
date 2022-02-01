glm_dep = dependency('glm')

kepler_inc = include_directories('.')

kepler_lib = static_library(
	'kepler',
	'transform.cpp',
	dependencies: [
		glm_dep,
		loguru_dep,
	],
)

kepler_dep = declare_dependency(
	include_directories: kepler_inc,
	link_with: kepler_lib,
)
