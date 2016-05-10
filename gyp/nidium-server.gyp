# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'targets': [{
        'target_name': '<(nidium_exec_name)',
        'type': 'executable',
        'product_dir': '<(nidium_exec_path)',
        'dependencies': [
            '<(nidium_nidiumcore_path)/gyp/nidiumcore.gyp:*',
            '<(nidium_network_path)/gyp/network.gyp:*'
        ],
        'include_dirs': [
            '<(nidium_network_path)',
            '<(third_party_path)/mozilla-central/js/src/dist/include/',
            '<(third_party_path)/http-parser/',
            '<(third_party_path)/linenoise/',
            '<(nidium_src_path)',
        ],
        'sources': [
            '<(nidium_src_path)/App/nidium_server_main.cpp',
            '<(nidium_src_path)/Server/Server.cpp',
            '<(nidium_src_path)/Server/Context.cpp',
            '<(nidium_src_path)/Server/REPL.cpp',
            '<(nidium_src_path)/Binding/JSSystem.cpp',

            '<(third_party_path)/libnostd/bsd/stdlib/setproctitle.c',
            '<(third_party_path)/linenoise/linenoise.c',

        ],
        'conditions': [
            ['nofork==1', {
                'defines':['NIDIUM_NO_FORK']
            }],
            ['OS=="linux"', {
                'cflags': [
                    '-fno-rtti',
                    # Not sure this makes any big difference. At least here.
                    # Could be better in network common.gypi
                    # See :http://stackoverflow.com/questions/4274804/query-on-ffunction-section-fdata-sections-options-of-gcc
                    #'-ffunction-sections',
                    #'-fdata-sections',
                    #'-fno-exceptions',
                    #'-freorder-blocks',
                    # Not sure this is interesting for us
                    # See : http://stackoverflow.com/questions/1942801/when-should-i-omit-the-frame-pointer
                    #'-fomit-frame-pointer',
                ],
            }],
        ],
    }]
}
