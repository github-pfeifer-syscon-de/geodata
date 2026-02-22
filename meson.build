project('geodata', 'c', 'cpp'
    , default_options: ['c_std=c11'
                       , 'cpp_std=c++20'
                       , 'buildtype=debugoptimized'
                       , 'warning_level=3']
    , license : 'GPL-3.0'
    , meson_version: '>= 1.9.0'
    , version: '0.1.4')
project_description = 'Common webMap (WMS) functions for glglobe'

thread_deps     = dependency('threads')
genericimg_deps = dependency('genericimg', version: '>= 0.4.0')
genericglm_deps = dependency('genericglm', version: '>= 0.3.1')
libsoup3_deps    = dependency('libsoup-3.0')


cc = meson.get_compiler('c')
disable_flags = [
    '-Wno-unused-parameter'  # limit the compiler noisiness
]
foreach cflag : disable_flags
    if cc.has_argument(cflag)
        add_project_arguments(cflag, language: ['c', 'cpp'])
    endif
endforeach

pkgdatadir = join_paths(get_option('prefix'), get_option('datadir'), meson.project_name())
# localedir = share/locale
localdir = join_paths(get_option('prefix'), get_option('localedir'))
add_project_arguments('-DPACKAGE_DATA_DIR="' + pkgdatadir + '"', language: 'cpp')
add_project_arguments('-DPACKAGE_LOCALE_DIR="' + localdir + '"', language: 'cpp')
public_headers = include_directories('include')

conf = configuration_data()
conf.set_quoted('VERSION', meson.project_version())     # Surround the version in quotes to make it a C string

subdir('include')
subdir('src')
deps = [ genericimg_deps
       , genericglm_deps
       , libsoup3_deps
       , thread_deps
       ]
# append resource.c
project_target = both_libraries(meson.project_name()
    , sources
    , install : true
    , dependencies: deps
    , gnu_symbol_visibility : 'default'         # visibility was hidden, but selected visible for compatibility
    , include_directories : public_headers
    , version : meson.project_version())
subdir('test')

pkg_mod = import('pkgconfig')
pkg_mod.generate(
    name : meson.project_name()
  , filebase : meson.project_name()
  , description : project_description
  , subdirs : meson.project_name()
  , libraries : project_target
  , requires : deps
)

# Make this library usable as a Meson subproject.
project_deps = declare_dependency(
  include_directories: public_headers,
  link_with : project_target
)
set_variable(meson.project_name() + '_dep', project_deps )