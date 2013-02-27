{
    'targets': [{
        'target_name': 'native',
        'type': 'none',
        'includes': [
            'common.gypi'
        ],
        'dependencies': [
            'network.gyp:nativenetwork',
            'interface.gyp:nativeinterface',
            'av.gyp:nativeav',
            'native.gyp:nativestudio',
            'app.gyp:nativeapp',
        ],
    }]
}
