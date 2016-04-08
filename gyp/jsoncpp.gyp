# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'targets': [
        {
            'target_name': 'jsoncpp',
            'type': 'static_library',
            'sources': [
                '../external/json/jsoncpp.cpp',
            ],
            'include_dirs': [
                '../external/json/'
            ],
            'direct_dependent_settings': {
                'include_dirs': [
                    '../external/json/'
                ]
            }
        },
    ],
}
