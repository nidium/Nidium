# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'targets': [{
        'conditions': [
            ['target_os=="android"', {
                'target_name': 'nidiumapp',
                'type': 'none',
                'dependencies': [
                    'libnidium_android.gyp:libnidium_android',
                ],
            }],
            ['nidium_target_type=="static_library"', {
                'target_name': 'nidiumapp',
                'type': 'none',
                'dependencies': [
                    # Gyp doesn't build libraries when building a library
                    # so add them here
                    'libnidium.gyp:libnidium',
                    'libnidiumcore.gyp:libnidiumcore',
                    '<(nidium_network_path)/gyp/network.gyp:network',
                    'nidium.gyp:<(nidium_output_name)',
                ],
            },
            {
                'target_name': 'nidiumapp',
                'type': 'none',
                'dependencies': [
                    'nidium.gyp:<(nidium_output_name)',
                ],
            }]
        ]
    }]
}

