#
# see https://mesonbuild.com/Gnome-module.html
gnome = import('gnome')

app_resources = gnome.compile_resources(
    'resources', meson.project_name() + '.gresources.xml'
    , source_dir: meson.current_source_dir()
)
