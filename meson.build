project(
  '42sh',
  'c',
  version : '1.0',
  default_options: [
    'c_std=c99',
    'warning_level=3',
    'werror=true',
  ],
)

criterion_dep = dependency('criterion')

cflags = ['-D_GNU_SOURCE', '-D_POSIX_C_SOURCE=200809L']
add_project_arguments(cflags, language: 'c')

incdirs = [
  include_directories('src'),
  include_directories('tests')
]

common = [
  'src/test.c'
]


main = executable(
    # name of the output executable
    '42sh',
    # source files for the executable
    common + [ 'src/main.c' ],
    include_directories: incdirs
)

tests = executable(
    'testsuite',
    common + [ 'tests/unit_tests.c' ],
    include_directories: incdirs,
    dependencies : criterion_dep
)