# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'targets': [{
        'target_name': 'libnidium_android',
        'type': 'shared_library',
        'dependencies': [
            'libnidium.gyp:*',
            'libnidiumcore.gyp:*',
            '<(nidium_network_path)/gyp/network.gyp:*',
            'third-party/SDL2-android.gyp:*',
        ],
        'include_dirs': [
            '<(third_party_path)/jnipp/'
        ],
        'includes': [
            'interface.gypi',
        ],
        'ldflags': [
            '-L../../<(nidium_output_third_party_path)',
            '-static-libstdc++',
            '-Wl,--build-id' # Needed for debugging with LLDB (file & line)
        ],
        'defines':[
            'JNIPP_USE_TYPE_TRAITS',
            'JNIPP_THREAD_LOCAL_PTHREAD',
        ],
        'link_settings': {
            'libraries!': [
                # Android has pthread & rt by default
                '-lpthread',
                '-lrt',
            ],
            'libraries': [
                "-lskia",
                "-lzip",
                "-ltranslator",
                "-ltranslator_lib",
                "-lpreprocessor",
                "-langle_common",
                "-lm",
                "-lEGL",
                "-lGLESv2",
                "-llog",
                "-landroid",
            ],
        },
    }]
}
