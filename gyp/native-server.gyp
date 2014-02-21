{
    'targets': [{
        'target_name': '<(native_exec_name)',
        'type': 'executable',
        'mac_bundle': 1,
        'product_dir': '<(native_exec_path)',
        'dependencies': [
            '<(native_nativejscore_path)/gyp/nativejscore.gyp:nativejscore'
        ],
        'include_dirs': [
            '<(native_network_path)',
            '<(third_party_path)/mozilla-central/js/src/dist/include/',
            '<(third_party_path)/http-parser/',
        ],
        'sources': [
            '<(native_src_path)/native_main.cpp',
            '<(native_src_path)/NativeServer.cpp',
        ],
        'conditions': [
            ['OS=="linux"', {
                'cflags': [
                    '-fno-rtti',
                    '-ffunction-sections',
                    '-fdata-sections',
                    '-fno-exceptions',
                    '-DTRIMMED',
                    '-freorder-blocks',
                    '-fomit-frame-pointer',
                    '-Wno-invalid-offsetof'
                ],
                'link_settings': {
                    'libraries': [
                        '-rdynamic',
                        '-Wl,--start-group',
                        '-lpthread',
                        '-lrt',
                        '-lz',

                        '-Wl,-Bstatic ',
                        '-lcares',
                        '-ljs_static',
                        '-lhttp_parser',
                        '-lnspr4',
                        '-Wl,-Bdynamic',

                        '-lleveldb',
                        '-Wl,--end-group',
                    ],
                },
            }],
            ['OS=="mac"', {
                "link_settings": {
                    'libraries': [
                        'libjs_static.a',
                        'libnspr4.a',
                        'libcares.a',
                        'libleveldb.a'
                    ],
                },
                "xcode_settings": {
                    'MACOSX_DEPLOYMENT_TARGET': [
                        '10.7'
                    ],
                    'LD_RUNPATH_SEARCH_PATHS': [
                        '@loader_path/../Frameworks'
                    ],
                    'OTHER_LDFLAGS': [ 
                        '-stdlib=libc++',
                    ],
                    'OTHER_CFLAGS': [ 
                        '-g',
                        '-O2',
                        '-Wall',
                    ],
                    'INFOPLIST_FILE': '<(native_resources_path)/osx/Info.plist',
                },
                'mac_bundle_resources': [
                    '<(native_resources_path)/osx/en.lproj/InfoPlist.strings',
                    '<(native_resources_path)/osx/en.lproj/MainMenu.xib',
                ],
                'postbuilds': [
                    {
                        'postbuild_name': 'Copy resources',
                        'action': [
                            'cp',
                            '-r',
                            '<(native_resources_path)/osx/',
                            '<(native_exec_path)/<(native_exec_name).app/Contents/Resources/'
                        ]
                    },
                    {
                        'postbuild_name': 'Increment build number',
                        'action': [
                            './osx/incbuild.sh',
                            '<(native_exec_path)/<(native_exec_name).app/Contents/'
                        ]
                    }
                ]
            }],
        ],
    }]
}
