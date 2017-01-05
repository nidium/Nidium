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
        'includes': [
            'interface.gypi',
        ],
        'ldflags': [
            '-L../../<(nidium_output_third_party_path)',
			'-static-libstdc++'
        ],
        'link_settings': {
            'libraries!': [
				# Android has pthread & rt by default
				'-lpthread',
				'-lrt',
			],
            'libraries': [
				"-Wl,--start-group",
				"-lskia_images",
				"-lskia_sfnt",
				"-lskia_skgpu",
				"-lskia_utils",
				"-lskia_ports",
				"-lskia_core",
				"-lskia_effects",
				"-lskia_opts",
				"-lskia_opts_neon",
				"-letc1",
				"-lgif",
				"-lwebp_dec",
				"-lwebp_enc",
				"-lwebp_utils",
				"-lwebp_dsp",
				"-lwebp_dsp_neon",
				"-lcpu_features",
				"-lexpat",
				"-lfreetype_static",
				"-Wl,--end-group",

				"-lzip",
				"-ltranslator",
				"-ltranslator_lib",
				"-lpreprocessor",
				"-langle_common",
				"-ljpeg",
				"-lpng",
				"-lm",
				"-lEGL",
				"-lGLESv2",
				"-llog",
				"-landroid",
            ],
        },
    }]
}
