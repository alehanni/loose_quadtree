
project('loose_quadtree', 'cpp', default_options : ['cpp_std=c++17'])

cpp = meson.get_compiler('cpp')

cmake = import('cmake')
deps = []

# build raylib
rl_opt_var = cmake.subproject_options()
rl_opt_var.add_cmake_defines({
    'BUILD_SHARED_LIBS': 'OFF',
    'USE_EXTERNAL_GLFW': 'OFF'
})

if host_machine.system() == 'emscripten'
    rl_opt_var.add_cmake_defines({'PLATFORM': 'Web'})
endif

raylib_sub_proj = cmake.subproject('raylib', options: rl_opt_var)
raylib_dep = raylib_sub_proj.dependency('raylib')

deps += raylib_dep

if host_machine.system() != 'emscripten'
    deps += raylib_sub_proj.dependency('glfw')
endif

# build flecs
flecs_proj = subproject('flecs')
flecs_dep = flecs_proj.get_variable('flecs_dep')
deps += flecs_dep

# build imgui
imgui_proj = subproject('imgui_build')
imgui_sources = imgui_proj.get_variable('sources')
imgui_inc = imgui_proj.get_variable('inc')

inc = [imgui_inc]
sources = [imgui_sources]

inc += include_directories('include')
subdir('src')

executable('demo', sources, include_directories : inc, dependencies : deps)

# build tests
subdir('tests')