# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'includes': ["../src/libapenetwork/gyp/config.gypi"],
    'variables' : {
        'nidium_version': '0.2',
        'nidium_webgl%': 1,
        'nidium_audio%': 1,
        'nidium_ui_console%': 1,
        'nidium_package_embed%': 0,
        'nidium_gl_debug%': 0,
        'nidium_embed_dir': '../src/Embed/',
        'nidium_embed_bin_header': '../src/nidium_embed_bin.h',
        'nidium_embed_bin': '../resources/embed.bin',

        'nidium_src_path': '../src/',
        'nidium_av_path': '<(nidium_src_path)/AV/',
        'nidium_app_path': '<(nidium_src_path)/Frontend/app/',
        'nidium_interface_path': '../src/Interface/',
        'nidium_network_path': '../src/libapenetwork/',
        'nidium_resources_path': '../resources/',
        'nidium_tools_path': '../tools/',
        'nidium_tests_path': '../tests/gunittest/',
        'nidium_exec_name': 'nidium',
        'nidium_exec_path': '../bin/',

        'nidium_output': '../build/',

        # Hack to workaround two gyp issues : 
        # - Variables defined in command line are not relativized (at all)
        #   https://code.google.com/p/gyp/issues/detail?id=72
        # - Variables defined in the top scope, cannot be referenced by another one
        'variables': {
            'third_party%': 'third-party',
            'nidium_output_third_party_path': '<(nidium_output)/third-party/',
            'nidium_tests_output_path': '<(nidium_output)/tests/',
        },
        'third_party_path': '<(DEPTH)/<(third_party)',
        'nidium_output_third_party_path': '<(nidium_output_third_party_path)',
        'nidium_tests_output_path': '<(nidium_tests_output_path)',

        'asan%': 0,
        'jemalloc%': 0,
        'profiler%': 0,
        'target_os%': '<(OS)',
        'mac_deployment_target': '10.7',
        'mac_sdk_version%': '10.11',

        'nidium_js_disable_window_global%': 1,

        # Crash reporter settings
        'nidium_enable_breakpad%': 0,
        'nidium_crash_collector_host': 'crash.nidium.com',
        'nidium_crash_collector_port': 80,
        'nidium_crash_collector_endpoint': '/submit',

        # Linux build only
        'nidium_use_gtk': 1,

        # Nidium server specifics
        'nofork%': 0
    },
}
