# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'includes': [
        '../network/gyp/common.gypi'
    ],
    'target_defaults': {
        'defines': [
            'NATIVE_VERSION_STR="<(native_version)"',
            'NIDIUM_NO_PRIVATE_DIR'
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
