{
    'targets': [
        {
            'target_name': 'jsoncpp',
            'type': 'static_library',
            'sources': [
                '../external/json/jsoncpp.cpp',
            ],
            'include_dirs': [
                '../external/json/',
            ],
            'direct_dependent_settings': {
                'include_dirs': [
                    '../external/json/',
                ]
            }
        },
    ],
}
