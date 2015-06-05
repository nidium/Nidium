{
    'includes': [
        '../nativejscore/network/gyp/config.gypi',
        '../nativejscore/network/gyp/common.gypi'
    ],
    'variables' : {
        'native_version': '0.1',

        'native_src_path': '../src/',
        'native_network_path': '../nativejscore/network/',
        'native_nativejscore_path': '../nativejscore/',
        'native_exec_name': 'nidium-server',
        'native_exec_path': '../dist/',

        'third_party_path%': '../third-party/',
        'native_output%': '../out/',
        'asan%': 0,
        'jemalloc%': 0,
        'nofork%': 0
    },
}
