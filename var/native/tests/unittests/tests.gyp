# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'targets': [{
        'target_name': 'nativejscore-unittests',
        'product_dir': '<(native_tests_output)',
        'type': 'executable',
        'dependencies': [
            '../../../../network/gyp/network-unittests.gyp:unittests-settings',
            '../../../../gyp/nativejscore.gyp:*',
            '../../../../network/gyp/network.gyp:*',
        ],
        'sources': [
            'path.cpp',
        ],
    }]
}
