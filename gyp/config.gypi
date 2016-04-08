# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'includes': [
        '../nativejscore/network/gyp/config.gypi',
    ],
    'variables' : {
        'native_version': '0.1',

        'native_src_path': '../src/',
        'native_network_path': '../nativejscore/network/',
        'native_nativejscore_path': '../nativejscore/',
        'native_exec_name': 'nidium-server',
        'native_exec_path': '../dist/',

        'third_party_path%': '../third-party/',
        'jemalloc%': 0,
        'nofork%': 0
    },
}
