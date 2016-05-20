# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'includes': [
        '../nativejscore/gyp/common.gypi',
    ],
    'target_defaults': {
        'defines': [
            'NIDIUM_PRODUCT_UI="1"',
            'NIDIUM_VERSION_STR="<(nidium_version)"',
            'NIDIUM_FRAMEWORK_STR="<(nidium_framework)"',
            'NIDIUM_BUILD="<!@(git rev-parse HEAD)"',
            'NIDIUM_CRASH_COLLECTOR_HOST="<(nidium_crash_collector_host)"',
            'NIDIUM_CRASH_COLLECTOR_PORT=<(nidium_crash_collector_port)',
            'NIDIUM_CRASH_COLLECTOR_ENDPOINT="<(nidium_crash_collector_endpoint)"',
#'UINT32_MAX=4294967295u',
#'_FILE_OFFSET_BITS=64',
#'_HAVE_SSL_SUPPORT',
#'NIDIUM_DONT_LOAD_FRAMEWORK'
        ],
        'default_configuration': 'Release',
        'configurations': {
            'Debug': {
                'product_dir': '<(nidium_output)/debug/',
                'defines': ['NATIVE_DEBUG', 'DEBUG', '_DEBUG'],
                'ldflags': [
                    # Skia need to be linked with its own libjpeg
                    # since libjpeg.a require .o files that are in a relative path 
                    # we must include skia gyp ouput directory
                    '-L<(third_party_path)/skia/out/Release/obj/gyp/',
                ],
            },
            'Release': {
                'product_dir': '<(nidium_output)/release/',
                'defines': [ 'NDEBUG'],
                'ldflags': [
                    # Skia need to be linked with his own libjpeg
                    # since libjpeg.a require .o files that are in a relative path 
                    # we must include skia gyp ouput directory
                    '-L<(third_party_path)/skia/out/Release/obj/gyp/',
                ],
            }
        },
        'conditions': [
            ['nidium_enable_breakpad==1', {
                'defines': [ 'NIDIUM_ENABLE_CRASHREPORTER' ],
            }],
            # XXX : Remove me once we switched to .nfs file for privates
            ['nidium_embed_private==1', {
                'defines': [
                    'NIDIUM_EMBED_PRIVATE="<(nidium_private_bin_header)"',
                ]
            }],
        ],
    },
}

