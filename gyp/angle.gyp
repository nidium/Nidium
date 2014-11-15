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
        # Can't use variable in includes 
        # https://code.google.com/p/gyp/wiki/InputFormatReference#Processing_Order
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
