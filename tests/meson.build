
test_deps = []

catch2_proj = subproject('Catch2')
catch2_with_main_dep = catch2_proj.get_variable('catch2_with_main_dep')
test_deps += catch2_with_main_dep

test_sources = files(
)

test_build = executable(
    'test_build',
    test_sources,
    include_directories: inc,
    dependencies: deps + test_deps
)

test('test_build', test_build)