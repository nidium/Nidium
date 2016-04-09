# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'includes': [
        '../nativejscore/gyp/common.gypi',
    ],
    'target_defaults': {
        'defines': [
            'NATIVE_VERSION_STR="<(native_version)"',
            'NATIVE_FRAMEWORK_STR="<(native_framework)"',
            'NATIVE_BUILD="<!@(git rev-parse HEAD)"',
            'NATIVE_CRASH_COLLECTOR_HOST="<(native_crash_collector_host)"',
            'NATIVE_CRASH_COLLECTOR_PORT=<(native_crash_collector_port)',
            'NATIVE_CRASH_COLLECTOR_ENDPOINT="<(native_crash_collector_endpoint)"',
#'UINT32_MAX=4294967295u',
#'_FILE_OFFSET_BITS=64',
#'_HAVE_SSL_SUPPORT',
#'NIDIUM_DONT_LOAD_FRAMEWORK'
        ],
        'default_configuration': 'Release',
        'configurations': {
            'Debug': {
                'product_dir': '<(native_output)/debug/',
                'defines': ['NATIVE_DEBUG', 'DEBUG', '_DEBUG'],
                'ldflags': [
                    # Skia need to be linked with its own libjpeg
                    # since libjpeg.a require .o files that are in a relative path 
                    # we must include skia gyp ouput directory
                    '-L<(third_party_path)/skia/out/Release/obj/gyp/',
                ],
            },
            'Release': {
                'product_dir': '<(native_output)/release/',
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
            ['native_enable_breakpad==1', {
                'defines': [ 'NATIVE_ENABLE_BREAKPAD' ],
            }],
            # XXX : Remove me once we switched to .nfs file for privates
            ['native_embed_private==1', {
                'defines': [
                    'NATIVE_EMBED_PRIVATE="<(native_private_bin_header)"',
                ]
            }],
        ],
    },
}

