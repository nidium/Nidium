{
    'targets': [
    {
        'target_name': 'nativejscore-unittests-settings',
        'type': 'none',
        'direct_dependent_settings': {
            'product_dir': '../tests/bin/',
            'include_dirs': [
                 '<(third_party_path)/gtest/include',
                '../..'
             ],
            'conditions': [
                ['OS=="linux"', {
                    "link_settings": {
                        'libraries': [
                            '-lgtest_main',
                            '-lgtest',
                        ]
                    }
                }],
                ['OS=="mac"', {
                    "link_settings": {
                        'libraries': [
                            'libgtest_main.a',
                            'libgtest.a',
                        ]
                    }
                }]
            ],
        },
    },
    {
        'target_name': 'nativejscore-unittests',
        'type': 'executable',
        'dependencies': [
            'nativejscore-unittests.gyp:nativejscore-unittests-settings',
            '../network/gyp/network.gyp:nativenetwork-includes',
            'nativejscore.gyp:nativejscore-includes',
            'nativejscore.gyp:nativejscore',
            'nativejscore.gyp:nativejscore-link',
            '../network/gyp/network.gyp:nativenetwork-link',
        ],
        'sources': [
            '../tests/unittest_0.cpp',
            '../tests/unittest_args.cpp',
            '../tests/unittest_db.cpp',
            '../tests/unittest_events.cpp',
            '../tests/unittest_file.cpp',
            #'../tests/unittest_filestream.cpp',
            '../tests/unittest_hash.cpp',
            #'../tests/unittest_http.cpp',
            '../tests/unittest_httplistener.cpp',
            '../tests/unittest_httpstream.cpp',
            '../tests/unittest_istreamer.cpp',
            '../tests/unittest_js.cpp',
            #'../tests/unittest_jsconsole.cpp',
            #'../tests/unittest_jsdebug.cpp',
            #'../tests/unittest_jsexposer.cpp',
            #'../tests/unittest_jsfileio.cpp',
            #'../tests/unittest_jsfs.cpp',
            #'../tests/unittest_jshttp.cpp',
            #'../tests/unittest_jshttplistener.cpp',
            #'../tests/unittest_jsmodules.cpp',
            #'../tests/unittest_jsprocess.cpp',
            #'../tests/unittest_jsprofiler.cpp',
            #'../tests/unittest_jssocket.cpp',
            #'../tests/unittest_jsstream.cpp',
            #'../tests/unittest_jsthread.cpp',
            #'../tests/unittest_jsutils.cpp',
            #'../tests/unittest_jswebsocket.cpp',
            '../tests/unittest_messages.cpp',
            '../tests/unittest_nfs.cpp',
            '../tests/unittest_nfsstream.cpp',
            '../tests/unittest_path.cpp',
            '../tests/unittest_sharedmessages.cpp',
            #'../tests/unittest_streaminterface.cpp',
            '../tests/unittest_taskmanager.cpp',
            '../tests/unittest_utils.cpp',
            '../tests/unittest_websocket.cpp',
        ],
    }
    ]
}


