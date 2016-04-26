{
    'includes': [
        '../nativejscore/gyp/common.gypi'
    ],
    'target_defaults': {
        'defines': [
            'NIDIUM_VERSION_STR="<(nidium_version)"',
            'NIDIUM_BUILD="<!@(git rev-parse HEAD)"',
            'NIDIUM_NO_PRIVATE_DIR'
        ],
        'configurations': {
            'Debug': {
                'product_dir': '<(nidium_output)/debug/',
            },
            'Release': {
                'product_dir': '<(nidium_output)/release/',
            }
	    }
    }
}
