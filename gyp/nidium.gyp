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
        'msvs_settings': {
            'VCLinkerTool': {
                'SubSystem': '2',  # windows app
                "AdditionalOptions": [
                ],
            },
        },
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
            ['OS=="win"', {
                'conditions': [
                    ['nidium_enable_breakpad==1', {
                        'dependencies': [
                            'crashreporter.gyp:nidium-crash-reporter',
                         ],
                    }],
                ],
                'link_settings': {
                    'libraries': [
            
                       'gif.lib',
                        '-lSDL2',

                        '-lskia',
                        '-langle_common',

                        '-ltranslator',
#                        '-ltranslator_lib',
                        '-lpreprocessor',

#                        '-lGL',
#                        '-lfreetype',
#                        '-lrt',
#                        '-ldl',
                        '-lzip',

                         'Compression.obj', 'Decimal.obj', #'Unified_cpp_mfbt_staticruntime0.obj',
                        'opengl32.lib',
                    #'usp10.lib',
                    #'kernel32.lib',
                    #'gdi32.lib',
                    #'winspool.lib',
                    #'comdlg32.lib',
                    #'advapi32.lib',
                    #'ole32.lib',
                    #'oleaut32.lib',
                    #'user32.lib',
                    #'uuid.lib',
                    #'odbc32.lib',
                    #'odbccp32.lib',
                    #'DelayImp.lib',
                    #'windowscodecs.lib',
                        'shell32.lib',
                        'shlwapi.lib',
#                         'winmm.lib',
#                         'imm32.lib',
#                         'delayimp.lib',
#                        'glu32.lib',
                    ],
                },
                'defines': [
                ],
                'sources': [
                    '<(nidium_app_path)/windows/main.cpp',
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
                'ldflags': [
                    '-rdynamic',
                ],
                'link_settings': {
                    'libraries': [
                        '-lskia',
                        '-ltranslator',
                        '-ltranslator_lib',
                        '-lpreprocessor',
                        '-langle_common',
                        '-lSDL2',
                        '-lGL',
                        '-lfreetype',
                        '-lrt',
                        '-ldl',
                        '-lzip',
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
                    'DEAD_CODE_STRIPPING': 'YES',
                    "OTHER_LDFLAGS": [
                        '-rdynamic',
                    ]
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
                        # On OSX if libjs_static.a is linked after libskia.a there is
                        # an issue with duplicate symbols (seems like gyp does not respect
                        # the link order)
                        'libjs_static.a',
                        'libskia.a',
                        'libtranslator.a',
                        'libtranslator_lib.a',
                        'libpreprocessor.a',
                        'libangle_common.a',
                        'libSDL2.a',
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
