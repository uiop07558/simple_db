# project('stupiddb', 'cpp')

# add_global_arguments('-Og', '-std=gnu++2b', language: 'cpp')

# boost = dependency('boost', method: 'pkg-config')

# src = [
#   './src/engine/bptree/test/test.cpp',
#   './src/engine/bptree/bptree.cpp',
#   './src/engine/page/page.cpp',
# ]

# executable('bptree_test', src, dependencies: [boost])

project(
    'stupiddb',
    'cpp',
    version: '1.0.0',
    default_options: [
        'cpp_std=c++23'
    ]
)

add_global_arguments('-O0' , '-g3', '-std=gnu++2b', language: 'cpp')

boost_dep = dependency('boost', method: 'pkg-config', required: false)  

if not boost_dep.found()
    boost_dep = dependency('boost', method: 'cmake', required: false)
endif

include_dirs = include_directories('src')

src = files(
    'src/unitybuild.cpp'
)

test_exe = executable(
    'test_bptree',
    sources: src,
    include_directories: include_dirs,
    dependencies: [boost_dep]
)

test(
    'bptree_test',
    test_exe
)