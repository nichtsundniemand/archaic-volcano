doxygen = find_program('doxygen', required: false)

awesomecss_proj = subproject('doxygen-awesome-css')
awesomecss_files = awesomecss_proj.get_variable('awesomecss_files')

awesomecss_base = '@0@/@1@'.format(meson.source_root(), awesomecss_files[0])
awesomecss_sidebaronly = '@0@/@1@'.format(meson.source_root(), awesomecss_files[1])

conf_data = configuration_data()
conf_data.set('DOXYGEN_INPUT', meson.source_root())
conf_data.set('DOXYGEN_OUTPUT', meson.current_build_dir())
conf_data.set('DOXYGEN_STYLESHEETS', '@0@ @1@'.format(awesomecss_base, awesomecss_sidebaronly))

doxyfile = configure_file(
  input: 'Doxyfile.in',
  output: 'Doxyfile',
  configuration: conf_data
)

custom_target('doc',
  build_by_default: true,
  command: [doxygen, doxyfile],
  output: ['html']
)
