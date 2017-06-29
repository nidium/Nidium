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
            },
            {
                'target_name': 'nidiumapp',
                'type': 'none',
                'dependencies': [
                    'nidium.gyp:<(nidium_lib_name)',
                ],
            }]
        ]
    }]
}

