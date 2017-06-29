# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'include_dirs': [
        '../src/',
        '<(nidium_network_path)'
    ],
    'sources': [
        '<(nidium_system_path)/OSImpl.cpp',
    ],
    'conditions': [
        ['target_os=="mac"', {
            'sources': [
                '<(nidium_system_path)/osx/OS.mm',
            ],
        }],
        ['target_os=="ios" or target_os=="tvos"', {
            'sources': [
                '<(nidium_system_path)/iOS/OS.mm',
            ]
        }],
        ['target_os=="linux"', {
            'sources': [
                '<(nidium_system_path)/linux/OS.cpp',
            ]
        }],
        ['target_os=="android"', {
            'sources': [

            ]
        }]
    ],
}

