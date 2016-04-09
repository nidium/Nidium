# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'targets': [{
        'target_name': 'dir2nvfs',
        'type': 'executable',
        'product_dir': '../tools/',
        'dependencies': [
            '<(native_network_path)/gyp/network.gyp:nativenetwork',
            '<(native_network_path)/gyp/network.gyp:nativenetwork-includes',
            '<(native_network_path)/gyp/network.gyp:nativenetwork-link',
            '<(native_nativejscore_path)/gyp/nativejscore.gyp:nativejscore',
            '<(native_nativejscore_path)/gyp/nativejscore.gyp:nativejscore-includes',
            '<(native_nativejscore_path)/gyp/nativejscore.gyp:nativejscore-link',
            '<(native_nativejscore_path)/gyp/jsoncpp.gyp:jsoncpp'
        ],
        'sources': [
            '<(native_src_path)/tools/dir2nvfs.cpp',
        ],
        'defines': [
            'DIR2NFS_OUTPUT="<(native_private_bin)"',
        ],
        "xcode_settings": {
            'OTHER_LDFLAGS': [ 
                '-stdlib=libc++',
            ],
        },
    }]
}
