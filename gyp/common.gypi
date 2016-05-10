# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'includes': [
        '../nativejscore/gyp/common.gypi'
    ],
    'target_defaults': {
        'defines': [
            'NIDIUM_PRODUCT_SERVER="1"',
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

