{
    'target_defaults': {
        'defines': [
            'NOMINMAX',
        ]
    },
    'variables': {
        'component': 'static_library'
    },
    'includes': [
        '../third-party/angle/src/build_angle.gypi'
    ],
    'targets': [{
        'target_name': 'angle',
        'type': 'none',
        'direct_dependent_settings': {
            'include_dirs': [
                '<(third_party_path)/angle/include/'
            ]
        }
    }]
}
