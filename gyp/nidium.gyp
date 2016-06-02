# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'targets': [{
        'target_name': '<(nidium_exec_name)',
        'type': 'executable',
        'mac_bundle': 1,
        'product_dir': '<(nidium_exec_path)',
        'dependencies': [
            'libnidium.gyp:*',
            'libnidiumcore.gyp:*',
            '<(nidium_network_path)/gyp/network.gyp:*',
        ],
        'includes': [
            'interface.gypi',
        ],
        'include_dirs': [
            '<(third_party_path)/SDL2/include/',
        ],
        'conditions': [
            ['nidium_enable_breakpad==1', {
                'dependencies': [
                    'crashreporter.gyp:nidium-crash-reporter',
                    'third-party/breakpad.gyp:*'
                 ],
                'defines': [
                    'NIDIUM_CRASH_COLLECTOR_HOST="<(nidium_crash_collector_host)"',
                    'NIDIUM_CRASH_COLLECTOR_PORT=<(nidium_crash_collector_port)',
                    'NIDIUM_CRASH_COLLECTOR_ENDPOINT="<(nidium_crash_collector_endpoint)"',
                ],
            }],
            ['OS=="linux"', {
                'conditions': [
                    ['nidium_enable_breakpad==1', {
                        'dependencies': [
                            'crashreporter.gyp:nidium-crash-reporter',
                         ],
                    }],
                ],
                'link_settings': {
                    'libraries': [
                        '-Wl,--start-group',
                        '-lskia_images',
                        '-lskia_sfnt',
                        '-lskia_skgpu',
                        '-lskia_utils',
                        '-lskia_ports',
                        '-lskia_core',
                        '-lskia_effects',
                        '-lskia_opts',
                        '-lskia_opts_ssse3',
                        '-Wl,--end-group',

                        '-lSDL2',
                        '-lGL',
                        '-lpng',
                        '-lfreetype',
                        '-lrt',
                        '-lbz2',
                        '-ldl',
                        '-lzip',
                        '-ljpeg',
                    ],
                },
                'include_dirs': [
                    '<(nidium_interface_path)/linux/',
                ],
                'sources': [
                    '<(nidium_app_path)/linux/main.cpp',
                ],
                #'actions': [{
                #    'action_name': 'strip',
                #    'inputs': '$(PRODUCT_DIR)/<(nidium_exec_name)',
                #    'outputs': '$(PRODUCT_DIR)/<(nidium_exec_name)',
                #    'action': ['strip', '$(PRODUCT_DIR)/<(nidium_exec_name)']
                #}]
            }],
            ['OS=="mac"', {
                # XXX : Dono why, but this has no effect
                "xcode_settings": {
                    'STRIP_INSTALLED_PRODUCT': 'YES',
                    'COPY_PHASE_STRIP': 'YES',
                    'DEBUGGING_SYMBOLS': 'NO',
                    'DEAD_CODE_STRIPPING': 'YES'
                },
                'conditions': [
                    ['nidium_enable_breakpad==1', {
                        'include_dirs': [
                            '<(third_party_path)/breakpad/src/client/mac/Framework/',
                            '<(third_party_path)/breakpad/src/client/apple/Framework/'
                        ],
                        "xcode_settings": {
                            'DEBUG_INFORMATION_FORMAT': 'dwarf-with-dsym',
                            'DEPLOYEMENT_POSTPROCESSING': 'YES',
                        }
                    }],
                ],
                "link_settings": {
                    'libraries': [
                        '$(SDKROOT)/System/Library/Frameworks/Carbon.framework',
                        '$(SDKROOT)/System/Library/Frameworks/ForceFeedback.framework',
                        '$(SDKROOT)/System/Library/Frameworks/IOKit.framework',
                        '$(SDKROOT)/System/Library/Frameworks/Cocoa.framework',
                        '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
                        'libSDL2.a',
                        'libskia_sfnt.a',
                        'libskia_opts_ssse3.a',
                        'libskia_opts.a',
                        'libskia_utils.a',
                        'libskia_ports.a',
                        'libskia_core.a',
                        'libskia_effects.a',
                        'libskia_images.a',
                        'libskia_skgpu.a',
                        'libzip.a',
                        '/usr/lib/libbz2.dylib',
                        '/usr/lib/libz.dylib',
                        'libiconv.dylib'
                    ],

                },
                "xcode_settings": {
                    'LD_RUNPATH_SEARCH_PATHS': [
                        '@loader_path/../Frameworks'
                    ],
                    'INFOPLIST_FILE': '<(nidium_resources_path)/osx/Info.plist',
                },
                'mac_bundle_resources': [
                    '<(nidium_resources_path)/osx/en.lproj/InfoPlist.strings',
                    '<(nidium_resources_path)/osx/en.lproj/MainMenu.xib',
                ],
                'include_dirs': [
                    '<(nidium_interface_path)/osx/',
                ],
                'sources': [
                    '<(nidium_app_path)/osx/main.mm',
                    '<(nidium_app_path)/osx/AppDelegate.mm',
                ],
                'postbuilds': [
                    #{
                    #    'postbuild_name': 'Copy Frameworks',
                    #    'action': [
                    #        'ditto',
                    #        '<(nidium_output)/third-party-libs/.libs/SDL2.framework/',
                    #        '<(nidium_exec_path)/<(nidium_exec_name).app/Contents/Frameworks/SDL2.framework'
                    #    ]
                    #},
                    {
                        'postbuild_name': 'Copy resources',
                        'action': [
                            'cp',
                            '-r',
                            '<(nidium_resources_path)/osx/',
                            '<(nidium_exec_path)/<(nidium_exec_name).app/Contents/Resources/'
                        ]
                    },
                    {
                        'postbuild_name': 'Increment build number',
                        'action': [
                            '<(nidium_tools_path)/osx_incbuild.sh',
                            '<(nidium_exec_path)/<(nidium_exec_name).app/Contents/'
                        ]
                    }
                ]
            }],
        ],
    }]
}

