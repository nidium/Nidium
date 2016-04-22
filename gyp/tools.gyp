{
    'targets': [{
        'target_name': 'dir2nvfs',
        'type': 'executable',
        'product_dir': '../tools/',
        'dependencies': [
            '<(native_network_path)/gyp/network.gyp:nativenetwork',
            '<(native_network_path)/gyp/network.gyp:nativenetwork-includes',
            '<(native_network_path)/gyp/network.gyp:nativenetwork-link',
            '<(native_nativejscore_path)/gyp/nidiumcore.gyp:nativejscore',
            '<(native_nativejscore_path)/gyp/nidiumcore.gyp:nativejscore-includes',
            '<(native_nativejscore_path)/gyp/nidiumcore.gyp:nativejscore-link',
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
