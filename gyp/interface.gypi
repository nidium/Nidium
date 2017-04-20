# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'include_dirs': [
        '../src/',
        '<(nidium_network_path)',
        '<(nidium_interface_path)/',
        '<(third_party_path)/libzip/lib/',
        '<(third_party_path)/SDL2/include/',
        '<(third_party_path)/c-ares/',
        '<(third_party_path)/angle/include/',
        '<(third_party_path)/rapidxml/',
        '<(third_party_path)/libnotify/',
        '<(third_party_path)/yoga/',
    ],
    'sources': [
        '<(nidium_interface_path)/UIInterface.cpp',
    ],
    'conditions': [
        ['OS=="mac"', {
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
            'defines': ['NIDIUM_USE_GTK'],
            'cflags': [
                '<!@(pkg-config --cflags gtk+-3.0)',
            ],
            'libraries': [
                '-lX11',
                '<!@(pkg-config --libs gtk+-3.0)',
                '-lnotify',
                '-lfontconfig',
            ],
        }],
        ['nidium_ui_console==0', {
            'defines': ['NIDIUM_DISABLE_UI_CONSOLE']
        }]
    ],
}

