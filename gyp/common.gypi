# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

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
        'configurations': {
            'Debug': {
                'product_dir': '<(native_output)/debug/',
            },
            'Release': {
                'product_dir': '<(native_output)/release/',
            }
        }
    }
}
