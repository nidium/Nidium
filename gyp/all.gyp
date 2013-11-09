{
    'targets': [{
        'target_name': 'native',
        'type': 'none',
        'dependencies': [
            #'third-party.gyp:third-party',
            #'network.gyp:nativenetwork',
            'interface.gyp:nativeinterface',
            'native.gyp:nativestudio',
            'app.gyp:nidium',
            '../nativejscore/gyp/nativejscore.gyp:nativejscore'
        ],
        'conditions': [
            ['native_audio==1', {
                'dependencies': [
                    'av.gyp:nativeav'
                 ]
            }],
            # Linux use his own crash reporter
            # on OSX breakpad default crash reporter is used
            ['native_enable_breakpad==1 and OS=="linux"', {
                'dependencies': [
                    'crashreporter.gyp:nidium-crash-reporter',
                 ]
            }]
         ]
    }]
}

