project(
	'libretro_volcano', 'cpp',
	version : '0.1',
	default_options : [
		'cpp_std=c++20',
		'warning_level=3',
	],
)

libretro_dep = dependency('libretro')
loguru_dep = dependency('loguru')

subdir('doc')
subdir('src')

retro_volcano = library(
	'retro_volcano',
	'libvolcano.cpp',
	dependencies : [
		libretro_dep,
		loguru_dep,
		volcano_dep,
	],
	include_directories: src_include,
	install : true
)

retroarch_prog = find_program('retroarch')

volcano_run = run_target(
	'run',
	command: [retroarch_prog, '-L', retro_volcano]
)
