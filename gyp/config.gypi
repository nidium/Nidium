{
    'includes': [
        '../nativejscore/network/gyp/config.gypi',
        '../nativejscore/network/gyp/common.gypi'
    ],
    'variables' : {
        'native_version': '0.1',
        'native_framework': 'falcon',
        'native_webgl%': 1,
        'native_audio%': 1,
        'native_embed_private%': 0,
        'native_private_dir': '../framework/dist/private/',
        'native_private_bin_header': '../src/Native_private_bin.h',
        'native_private_bin': '../resources/private.bin',

        'native_src_path': '../src/',
        'native_av_path': '../av/',
        'native_app_path': '../app/',
        'native_interface_path': '../interface/',
        'native_network_path': '../nativejscore/network/',
        'native_nativejscore_path': '../nativejscore/',
        'native_resources_path': '../resources/',
        'native_tools_path': '../tools/',
        'native_exec_name': 'nidium',
        'native_exec_path': '../framework/dist/',

        'native_output%': '../out/',
        'third_party_path': '../third-party/',
#'asan%': 0,

        'native_interface': 'auto', 

        # Crash reporter settings
        'native_enable_breakpad%': 0,
        'native_crash_collector_host': 'crash.nidium.com',
        'native_crash_collector_port': 80,
        'native_crash_collector_endpoint': '/submit',

        # Linux build only
        'native_use_gtk': 1,
        'native_use_qt': 0,
    },
}
