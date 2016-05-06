# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'targets': [{
        'target_name': 'nidiuminterface',
        'type': 'static_library',
        'include_dirs': [
            '../src/',
            '<(nidium_network_path)',
            '<(nidium_nidiumcore_path)/',
            '<(nidium_interface_path)/',
            '<(third_party_path)/libzip/lib/',
            '<(third_party_path)/SDL2/include/',
            '<(third_party_path)/c-ares/',
            '<(third_party_path)/angle/include/',
            '<(third_party_path)/rapidxml',
        ],
        'dependencies': [
            '<(nidium_network_path)/gyp/network.gyp:network-includes',
            '<(nidium_nidiumcore_path)/gyp/nidiumcore.gyp:nidiumcore-includes',
            '<(nidium_nidiumcore_path)/gyp/jsoncpp.gyp:jsoncpp',
        ],
        'sources': [
            '<(nidium_interface_path)/UIInterface.cpp',
        ],
        'conditions': [
            ['OS=="mac"', {
                'xcode_settings': {
                    'OTHER_CFLAGS': [
                        '-fvisibility=hidden',
                        '-Wno-invalid-offsetof'
                    ],
                },
                'sources': [
                    '<(nidium_interface_path)/osx/CocoaUIInterface.mm',
                    '<(nidium_interface_path)/osx/UIConsole.mm',
                    '<(nidium_interface_path)/osx/System.mm',
                    '<(nidium_interface_path)/osx/DragNSView.mm',
                ],
            }],
            ['OS=="linux"', {
                'sources': [
                    '<(nidium_interface_path)/linux/X11UIInterface.cpp',
                    '<(nidium_interface_path)/linux/System.cpp',
                ],
                'include_dirs': [
                    '<(nidium_interface_path)/linux/',
                ],
                'cflags': [
                    '-Wno-c++0x-extensions',
                    '-std=c++11',
                    '-Wno-invalid-offsetof'
                ],
                'conditions': [
                    ['nidium_use_gtk==1', {
                        'defines': ['NIDIUM_USE_GTK'],
                        'cflags': [
                            '<!@(pkg-config --cflags gtk+-2.0)',
                        ],
                    }],
                    ['nidium_use_qt==1', {
                        'defines': ['NIDIUM_USE_QT'],
                        'cflags': [
                            '<!@(pkg-config --cflags QtCore QtGui)'
                        ],
                    }]
                ],
                'direct_dependent_settings': {
                    'libraries': [
                        '-lX11',
                    ],
                    'conditions': [
                        ['nidium_use_gtk==1', {
                            'defines': ['NIDIUM_USE_GTK'],
                            'libraries': [
                                '<!@(pkg-config --libs gtk+-2.0)',
                            ]
                        }],
                        ['nidium_use_qt==1', {
                            'defines': ['NIDIUM_USE_QT'],
                            'libraries': [
                                '<!@(pkg-config --libs QtCore QtGui)'
                            ]
                        }]
                    ]
                },
            }],
        ],
    }],
}

