{
    'includes': [
        '../network/gyp/common.gypi'
    ],
    'target_defaults': {
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
    },
}
