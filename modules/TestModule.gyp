{
    'targets': [{
        'target_name': 'testmodule',
        'type': 'shared_library',
        'include_dirs': [
            '<(native_src_path)',
            '<(third_party_path)/mozilla-central/js/src/dist/include/',
        ],
        'sources': [ 'TestModule.cpp' ]
    }]
}
