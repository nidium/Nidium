{
    'targets': [{
        'target_name': 'nativejscore-unittests',
        'product_dir': '<(native_tests_output)',
        'type': 'executable',
        'dependencies': [
            '../../../../network/gyp/network-unittests.gyp:unittests-settings',
            '../../../../gyp/nativejscore.gyp:*',
            '../../../../network/gyp/network.gyp:*',
        ],
        'sources': [
            'path.cpp',
        ],
    }]
}
