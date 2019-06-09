
project('sigma-acpi-driver', 'cpp', default_options : ['cpp_std=c++17'], version: '0.1.0')

executable_name = 'acpi'

conf_data = configuration_data()
conf_data.set('version', meson.project_version())
conf_data.set('executable_name', executable_name)
configure_file(input: 'include/config.h.in', output: 'config.h', configuration: conf_data)

deps = []

sources = ['source/main.cpp', 'source/linux/dump.cpp']

includes = include_directories('include')

executable(executable_name, sources, dependencies: deps, include_directories: includes)
