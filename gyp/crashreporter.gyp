# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'targets': [{
        'target_name': 'nidium-crash-reporter',
        'type': 'executable',
        'product_dir': '../framework/dist/',
        'sources': [
            '<(native_app_path)/linux/NativeCrashReporter.c',
        ],
    }],
}

