{
    'includes': [
        '../network/gyp/common.gypi'
    ],
    'target_defaults': {
        'defines': [
            'NATIVE_VERSION_STR="<(native_version)"',
            'NATIVE_NO_PRIVATE_DIR'
        ],

        'cflags_cc': [
            '-std=c++11'
        ],

        'xcode_settings': {
            "OTHER_LDFLAGS": [
                '-stdlib=libc++'
            ],
            'OTHER_CPLUSPLUSFLAGS': [ 
                '-std=c++11',
                '-stdlib=libc++',
            ],
        },
        'configurations': {
            'Debug': {
                'defines': ['NATIVE_DEBUG', 'DEBUG', '_DEBUG'],
            },
            'Release': {
                'defines': ['NDEBUG'],
            }
        },
    },
}
