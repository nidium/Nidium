{
    'includes': [
        '../nativejscore/gyp/common.gypi'
    ],
    'target_defaults': {
        'defines': [
            'NATIVE_VERSION_STR="<(native_version)"',
            'NATIVE_BUILD="<!@(git rev-parse HEAD)"',
            'NATIVE_NO_PRIVATE_DIR'
        ],
        'default_configuration': 'Release',
        'configurations': {
            'Debug': {
                'product_dir': '<(native_output)/debug/',
                'defines': ['NATIVE_DEBUG', 'DEBUG', '_DEBUG', 'JS_DEBUG'],
            },
            'Release': {
                'product_dir': '<(native_output)/release/',
                'defines': ['NDEBUG'],
            }
        },
    },
}
