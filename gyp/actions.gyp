# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'targets': [{
        'target_name': 'generate-private',
        'type': 'none',
        'actions': [
            {
                'action_name': 'Generating Private',
                'inputs': [
                    # Any change to dir2nvfs will trigger the action
                    '<(nidium_src_path)/tools/dir2nvfs.cpp',
                    '<(nidium_tools_path)dir2nvfs',
                    # If I use find command inside a variable, output is not 
                    # considered as multiple files using it in inputs works fine
                    '<!@(find <(nidium_private_dir) -type f)',
                ],
                'outputs': [
                    '<(nidium_private_bin)'
                ],
                'action': ['<@(nidium_tools_path)dir2nvfs', '<(nidium_private_dir)', '<@(_outputs)'],
                'process_outputs_as_sources': 1
            },
            {
                'action_name': 'Converting Private',
                'inputs': [
                    '<(nidium_private_bin)'
                ],
                'outputs': [
                    '<(nidium_private_bin_header)'
                ],
                'action': ['xxd', '-i', '<@(_inputs)', '<@(_outputs)'],
                'process_outputs_as_sources': 1
            },
            {
                'action_name': 'Renaming Private C Header',
                'inputs': [
                    '<(nidium_private_bin_header)'
                ],
                'outputs': [
                    'dummyFile' # We need to set something here, otherwise action is not executed (OSX)
                ],
                'conditions': [
                    ['OS=="mac"', {
                        'action': ['sed', '-i', '""', '-e', '1s/.*/unsigned char private_bin[] = {/', '<@(_inputs)'],
                    }],
                    ['OS=="linux"', {
                        'action': ['sed', '-i', '-e', '1s/.*/unsigned char private_bin[] = {/', '<@(_inputs)'],
                    }]
                ],
                'process_outputs_as_sources': 1
            }
        ]
    }]
}

