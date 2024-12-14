project(
    'simple_db',
    'cpp',
    version: '1.0.0',
    default_options: {
        'cpp_std': 'c++23',
        # 'cpp_args': [
        #     'O0', '-g3',
        #     '-fsanitize=address,undefined'
        # ]
    }
)

add_global_arguments('-O0' , '-g3', '-std=gnu++2b',  language: 'cpp')
add_global_link_arguments('-O0' , '-g3', '-std=gnu++2b', language: 'cpp')

boost_dep = dependency('boost', modules: ['thread'], method: 'pkg-config', required: false)  

if not boost_dep.found()
    boost_dep = dependency('boost', modules: ['thread'], method: 'cmake', required: false)
endif

json_dep = dependency('nlohmann_json', method: 'pkg-config', required: false)  

if not json_dep.found()
    json_dep = dependency('nlohmann_json', method: 'cmake', required: false)
endif

# subproject('abseil-cpp')
# subproject('c-ares')
# subproject('openssl')
# subproject('protobuf')
# subproject('re2')

# grpc_dep = subproject('grpc').get_variable('grpc_dep')

include_dirs = include_directories('src')

test_pager_src = files(
    'src/test_pager_unitybuild.cpp'
)

test_pager_exe = executable(
    'test_pager',
    sources: test_pager_src,
    include_directories: include_dirs,
    dependencies: [boost_dep]
)

service_src = files(
    'src/service_unitybuild.cpp'
)

service_exe = executable(
    'simple_db',
    sources: service_src,
    include_directories: include_dirs,
    dependencies: [boost_dep, json_dep]
)