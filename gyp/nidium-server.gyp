# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'targets': [{
        'target_name': 'nidium-server',
        'type': 'executable',
        'product_dir': '<(nidium_exec_path)',
        'dependencies': [
            'libnidiumcore.gyp:*',
            '<(nidium_network_path)/gyp/network.gyp:*'
        ],
        'include_dirs': [
            '<(third_party_path)/linenoise/',
            '<(nidium_src_path)',
        ],
        'cflags': [
             "-Wno-expansion-to-defined",
        ],
        'sources': [
            '<(nidium_src_path)/Server/app/main.cpp',
            '<(nidium_src_path)/Server/Server.cpp',
            '<(nidium_src_path)/Server/Context.cpp',
            '<(nidium_src_path)/Server/REPL.cpp',

            '<(third_party_path)/setproctitle/setproctitle.c',
            '<(third_party_path)/linenoise/linenoise.c',
        ],
        'defines':[
            'LINENOISE_INTERRUPTIBLE',
        ],
        'conditions': [
            ['OS=="linux"', {
                'ldflags': [
                    '-rdynamic',
                ],
            }],
            ['OS=="mac"', {
                "xcode_settings": {
                    "OTHER_LDFLAGS": [
                        '-rdynamic',
                    ]
                },
            }],
            ['nofork==1', {
                'defines':['NIDIUM_NO_FORK']
            }],
        ],
    }]
}
