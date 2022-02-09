project(
	'libretro_volcano', 'cpp',
	version : '0.1',
	default_options : [
		'cpp_std=c++20',
		'warning_level=3',
	],
)

add_project_arguments(
	'-Wno-missing-field-initializers',
	language: 'cpp'
)

loguru_dep = dependency('loguru')

subdir('doc')
subdir('src')

retroarch_prog = find_program('retroarch')

volcano_run = run_target(
	'run',
	command: [retroarch_prog, '-L', retro_volcano]
)
