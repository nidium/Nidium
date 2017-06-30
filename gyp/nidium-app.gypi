# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'mac_bundle': 1,
    'conditions': [
        ['target_os=="linux"', {
            'ldflags': [
                '-rdynamic',
            ],
            'link_settings': {
                'libraries': [
                    '-lskia',
                    '-ltranslator',
                    '-ltranslator_lib',
                    '-lpreprocessor',
                    '-langle_common',
                    '-lSDL2',
                    '-lGL',
                    '-lfreetype',
                    '-lrt',
                    '-ldl',
                    '-lzip',
                ],
            },
        }],
        ['target_os=="mac"', {
            # XXX : Dono why, but this has no effect
            "xcode_settings": {
                'STRIP_INSTALLED_PRODUCT': 'YES',
                'COPY_PHASE_STRIP': 'YES',
                'DEBUGGING_SYMBOLS': 'NO',
                'DEAD_CODE_STRIPPING': 'YES',
                "OTHER_LDFLAGS": [
                    '-rdynamic',
                ],
                'LD_RUNPATH_SEARCH_PATHS': [
                    '@loader_path/../Frameworks'
                ],
                'INFOPLIST_FILE': '<(nidium_resources_path)/osx/Info.plist',
            },
            "link_settings": {
                'libraries': [
                    '$(SDKROOT)/System/Library/Frameworks/Carbon.framework',
                    '$(SDKROOT)/System/Library/Frameworks/ForceFeedback.framework',
                    '$(SDKROOT)/System/Library/Frameworks/IOKit.framework',
                    '$(SDKROOT)/System/Library/Frameworks/Cocoa.framework',
                    '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
                    # On OSX if libjs_static.a is linked after libskia.a there is
                    # an issue with duplicate symbols (seems like gyp does not respect
                    # the link order)
                    'libjs_static.a',
                    'libskia.a',
                    'libtranslator.a',
                    'libtranslator_lib.a',
                    'libpreprocessor.a',
                    'libangle_common.a',
                    'libSDL2.a',
                    'libzip.a',
                    '/usr/lib/libbz2.dylib',
                    '/usr/lib/libz.dylib',
                    'libiconv.dylib'
                ],

            },
            'mac_bundle_resources': [
                '<(nidium_resources_path)/osx/en.lproj/InfoPlist.strings',
                '<(nidium_resources_path)/osx/en.lproj/MainMenu.xib',
            ],
            'postbuilds': [
                {
                    'postbuild_name': 'Copy resources',
                    'action': [
                        'cp',
                        '-r',
                        '<(nidium_resources_path)/osx/',
                        '<(nidium_output_path)/<(nidium_output_name).app/Contents/Resources/'
                    ]
                },
                {
                    'postbuild_name': 'Increment build number',
                    'action': [
                        '<(nidium_tools_path)/osx_incbuild.sh',
                        '<(nidium_output_path)/<(nidium_output_name).app/Contents/'
                    ]
                }
            ]
        }],
        ['target_os=="ios" or target_os=="tvos"', {
            "link_settings": {
                'libraries': [
                    'OpenGLES.framework',
                    'CoreGraphics.framework',
                    'QuartzCore.framework',
                    'CoreFoundation.framework',
                    'CoreText.framework',
                    'CoreData.framework',
                    'Foundation.framework',
                    'UIKit.framework',
                    'GameController.framework',
                    # On OSX if libjs_static.a is linked after libskia.a there is
                    # an issue with duplicate symbols (seems like gyp does not respect
                    # the link order)
                    'libjs_static.a',
                    'libskia.a',
                    'libtranslator.a',
                    'libtranslator_lib.a',
                    'libpreprocessor.a',
                    'libangle_common.a',
                    'libzip.a',
                    'libbz2.dylib',
                    'libz.dylib',
                    'libiconv.dylib',
                    'libSDL2.a',
                ],

            },
            'postbuilds': [
                {
                    'postbuild_name': 'Copy Embed',
                    'action': [
                        'cp',
                        '-r',
                        '<(nidium_embed_dir)',
                        '<(nidium_output_path)/<(nidium_output_name).app/Embed/'
                    ]
                }
            ],
            "xcode_settings": {
                'LD_RUNPATH_SEARCH_PATHS': [
                    '@loader_path/../Frameworks'
                ],
                'CODE_SIGN_IDENTITY': 'iPhone Developer',
                'PROVISIONING_PROFILE': '',
                'PRODUCT_BUNDLE_IDENTIFIER': 'com.nidium.ios',
                'DEVELOPMENT_TEAM': '',
                'INFOPLIST_FILE': '<(nidium_resources_path)/iOS/Info.plist',
            },
        }],
        ['target_os=="ios"', {
            "link_settings": {
                'libraries': [
                    'CoreMotion.framework',
                ],
            },
        }],
    ],
}
