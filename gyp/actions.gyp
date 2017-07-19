# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'targets': [{
        'target_name': 'generate-embed',
        'type': 'none',
        'actions': [
            {
                'action_name': 'Generating embed',
                'inputs': [
                    # Any change to dir2nvfs will trigger the action
                    '<(nidium_src_path)/Tools/dir2nvfs.cpp',
                    '<(nidium_tools_path)dir2nvfs',
                    # Using find command inside a variable, output is not
                    # considered as multiple files. Using it in inputs works fine
                    '<!@(find <(nidium_embed_dir) -type f |sed \'s/.*/"&"/\')',
                ],
                'outputs': [
                    '<(nidium_embed_bin)'
                ],
                'action': ['<@(nidium_tools_path)dir2nvfs', '<(nidium_embed_dir)', '<@(_outputs)'],
                'process_outputs_as_sources': 1
            },
            {
                'action_name': 'Converting embed',
                'inputs': [
                    '<(nidium_embed_bin)'
                ],
                'outputs': [
                    '<(nidium_embed_bin_header)'
                ],
                'action': ['xxd', '-i', '<@(_inputs)', '<@(_outputs)'],
                'process_outputs_as_sources': 1
            },
            {
                'action_name': 'Renaming embed C Header',
                'inputs': [
                    '<(nidium_embed_bin_header)'
                ],
                'outputs': [
                    'dummyFile' # We need to set something here, otherwise action is not executed (OSX)
                ],
                'conditions': [
                    ['OS=="mac"', {
                        'action': ['sed', '-i', '""', '-e', '1s/.*/unsigned char embed_bin[] = {/', '<@(_inputs)'],
                    }],
                    ['OS=="linux"', {
                        'action': ['sed', '-i', '-e', '1s/.*/unsigned char embed_bin[] = {/', '<@(_inputs)'],
                    }]
                ],
                'process_outputs_as_sources': 1
            }
        ]
    }]
}

