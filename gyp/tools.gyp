# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'targets': [{
        'target_name': 'dir2nvfs',
        'type': 'executable',
        'product_dir': '../tools/',
        'dependencies': [
            '<(nidium_network_path)/gyp/network.gyp:network',
            '<(nidium_network_path)/gyp/network.gyp:network-includes',
            '<(nidium_network_path)/gyp/network.gyp:network-link',
            'libnidiumcore.gyp:libnidiumcore',
            'libnidiumcore.gyp:libnidiumcore-includes',
            'libnidiumcore.gyp:libnidiumcore-link',
        ],
        'sources': [
            '<(nidium_src_path)/Tools/dir2nvfs.cpp',
        ],
        'defines': [
            'DIR2NFS_OUTPUT="<(nidium_private_bin)"',
        ],
        "xcode_settings": {
            'OTHER_LDFLAGS': [
                '-stdlib=libc++',
            ],
        },
    }]
}
