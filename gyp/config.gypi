{
    'includes': [
        '../nativejscore/network/gyp/config.gypi',
        '../nativejscore/network/gyp/common.gypi'
    ],
    'variables' : {
        'nidium_version': '0.1',
        'nidium_framework': 'falcon',
        'nidium_webgl%': 1,
        'nidium_audio%': 1,
        'nidium_embed_private%': 0,
        'nidium_private_dir': '../framework/dist/private/',
        'nidium_private_bin_header': '../src/Native_private_bin.h',
        'nidium_private_bin': '../resources/private.bin',

        'nidium_src_path': '../src/',
        'nidium_av_path': '../av/',
        'nidium_app_path': '../app/',
        'nidium_interface_path': '../interface/',
        'nidium_network_path': '../nativejscore/network/',
        'nidium_nidiumcore_path': '../nativejscore/',
        'nidium_resources_path': '../resources/',
        'nidium_tools_path': '../tools/',
        'nidium_exec_name': 'nidium',
        'nidium_exec_path': '../framework/dist/',

        'nidium_output%': '../out/',
        'third_party_path': '../third-party/',
#'asan%': 0,

        'nidium_interface': 'auto',

        # Crash reporter settings
        'nidium_enable_breakpad%': 0,
        'nidium_crash_collector_host': 'crash.nidium.com',
        'nidium_crash_collector_port': 80,
        'nidium_crash_collector_endpoint': '/submit',

        # Linux build only
        'nidium_use_gtk': 1,
        'nidium_use_qt': 0,
    },
}
