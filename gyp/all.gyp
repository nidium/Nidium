{
    'targets': [{
        'target_name': 'native',
        'type': 'none',
        'includes': [
            'common.gypi'
        ],
        'dependencies': [
            #'third-party.gyp:third-party',
            'network.gyp:nativenetwork',
            'interface.gyp:nativeinterface',
            'av.gyp:nativeav',
            'native.gyp:nativestudio',
            'app.gyp:nativeapp',
        ],
    }]
}
