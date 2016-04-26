{
    'includes': [
        '../nativejscore/network/gyp/config.gypi',
    ],
    'variables' : {
        'nidium_version': '0.1',

        'nidium_src_path': '../src/',
        'nidium_network_path': '../nativejscore/network/',
        'nidium_nidiumcore_path': '../nativejscore/',
        'nidium_exec_name': 'nidium-server',
        'nidium_exec_path': '../dist/',

        'third_party_path%': '../third-party/',
        'jemalloc%': 0,
        'nofork%': 0
    },
}
