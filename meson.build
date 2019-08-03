
project('sigma-acpi-driver', 'cpp', version: '0.1.0')

executable_name = 'acpi'

conf_data = configuration_data()
conf_data.set('version', meson.project_version())
conf_data.set('executable_name', executable_name)
configure_file(input: 'include/config.h.in', output: 'config.h', configuration: conf_data)

deps = []

sources = files('source/main.cpp',
                'source/linux/dump.cpp',
                'source/acpi/tables.cpp',
                'source/acpi/aml/parse.cpp',
                'source/acpi/aml/aml_types.cpp')

includes = include_directories('include')

cpp_args = ['-std=c++2a']

add_global_arguments(cpp_args, language: 'cpp')

executable(executable_name, sources, dependencies: deps, include_directories: includes)
