glslc_prog = find_program('glslc')

shaders = files(
	'triangle.vert',
	'triangle.frag',
)

shader_targets = []
foreach shader: shaders
  shader_targets += custom_target(
    'shader-header @0@'.format(shader),
    command : [glslc_prog, '-c', '-mfmt=c', '@INPUT@', '-o',  '@OUTPUT@'],
    input : shader,
    output : '@PLAINNAME@.inc',
  )
endforeach

shader_dep = declare_dependency(
	sources: shader_targets,
)
