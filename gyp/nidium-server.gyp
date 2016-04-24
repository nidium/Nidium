{
    'targets': [{
        'target_name': '<(native_exec_name)',
        'type': 'executable',
        'product_dir': '<(native_exec_path)',
        'dependencies': [
            '<(native_nativejscore_path)/gyp/nidiumcore.gyp:*',
            '<(native_network_path)/gyp/network.gyp:*'
        ],
        'include_dirs': [
            '<(native_network_path)',
            '<(third_party_path)/mozilla-central/js/src/dist/include/',
            '<(third_party_path)/http-parser/',
            '<(native_src_path)',
        ],
        'sources': [
            '<(native_src_path)/Server/nidium_server_main.cpp',
            '<(native_src_path)/Server/Server.cpp',
            '<(native_src_path)/Server/Context.cpp',
            '<(native_src_path)/Server/REPL.cpp',
            '<(native_src_path)/Binding/JSSystem.cpp',
            '<(native_src_path)/Binding/JSConsole.cpp',
            '<(native_src_path)/external/setproctitle.c',
            '<(native_src_path)/external/linenoise.c',

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
