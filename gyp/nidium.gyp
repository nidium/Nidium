# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'targets': [{
        'target_name': '<(nidium_output_name)',
        'type': '<(nidium_target_type)',
        'product_dir': '<(nidium_output_path)',
        'dependencies': [
            'libnidium.gyp:*',
            'libnidiumcore.gyp:*',
            '<(nidium_network_path)/gyp/network.gyp:*',
        ],
        'includes': [
            'interface.gypi',
        ],
        'include_dirs': [
            '<(third_party_path)/SDL2/include/',
        ],
        'conditions': [
            # If we are building an executable, include nidium-app.gypi that contains
            # all the build/link flags needed for the application
            ['nidium_target_type=="executable"', {
                'includes': [
                    'nidium-app.gypi',
                ],
            }],
            ['nidium_enable_breakpad==1', {
                'dependencies': [
                    'crashreporter.gyp:nidium-crash-reporter',
                    'third-party/breakpad.gyp:*'
                 ],
                'defines': [
                    'NIDIUM_CRASH_COLLECTOR_HOST="<(nidium_crash_collector_host)"',
                    'NIDIUM_CRASH_COLLECTOR_PORT=<(nidium_crash_collector_port)',
                    'NIDIUM_CRASH_COLLECTOR_ENDPOINT="<(nidium_crash_collector_endpoint)"',
                ],
            }],
            ['target_os=="linux"', {
                'conditions': [
                    ['nidium_enable_breakpad==1', {
                        'dependencies': [
                            'crashreporter.gyp:nidium-crash-reporter',
                         ],
                    }],
                ],
                'include_dirs': [
                    '<(nidium_interface_path)/linux/',
                ],
                'sources': [
                    '<(nidium_app_path)/linux/main.cpp',
                ],
            }],
            ['target_os=="mac"', {
                'conditions': [
                    ['nidium_enable_breakpad==1', {
                        'include_dirs': [
                            '<(third_party_path)/breakpad/src/client/mac/Framework/',
                            '<(third_party_path)/breakpad/src/client/apple/Framework/'
                        ],
                        "xcode_settings": {
                            'DEBUG_INFORMATION_FORMAT': 'dwarf-with-dsym',
                            'DEPLOYEMENT_POSTPROCESSING': 'YES',
                        }
                    }],
                ],
                'include_dirs': [
                    '<(nidium_interface_path)/osx/',
                ],
                'sources': [
                    '<(nidium_app_path)/osx/main.mm',
                    '<(nidium_app_path)/osx/AppDelegate.mm',
                ],
            }],
            ['target_os=="ios" or target_os=="tvos"', {
                'include_dirs': [
                    '<(nidium_interface_path)/ios/',
                ],
                'sources': [
                    '<(nidium_app_path)/ios/main.mm',
                ],
            }],
        ],
    }]
}
