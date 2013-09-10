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
            }]
         ]
    }]
}

