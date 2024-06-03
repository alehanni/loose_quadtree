project('imgui', 'cpp')

sources = files(
    'imgui/imgui.cpp',
    'imgui/imgui_demo.cpp',
    'imgui/imgui_draw.cpp',
    'imgui/imgui_tables.cpp',
    'imgui/imgui_widgets.cpp',
    'imgui/backends/imgui_impl_glfw.cpp',
    'imgui/backends/imgui_impl_opengl3.cpp')

inc = include_directories('imgui', 'imgui/backends')

#libimgui = static_library('imgui', sources, include_directories : inc)

#imgui_dep = declare_dependency(include_directories : inc, link_with : libimgui)