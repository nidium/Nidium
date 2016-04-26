{
    'targets': [{
        'target_name': 'dir2nvfs',
        'type': 'executable',
        'product_dir': '../tools/',
        'dependencies': [
            '<(nidium_network_path)/gyp/network.gyp:network',
            '<(nidium_network_path)/gyp/network.gyp:network-includes',
            '<(nidium_network_path)/gyp/network.gyp:network-link',
            '<(nidium_nidiumcore_path)/gyp/nidiumcore.gyp:nidiumcore',
            '<(nidium_nidiumcore_path)/gyp/nidiumcore.gyp:nidiumcore-includes',
            '<(nidium_nidiumcore_path)/gyp/nidiumcore.gyp:nidiumcore-link',
            '<(nidium_nidiumcore_path)/gyp/jsoncpp.gyp:jsoncpp'
        ],
        'sources': [
            '<(nidium_src_path)/tools/dir2nvfs.cpp',
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
