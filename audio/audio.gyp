{
    'targets': [{
        'target_name': 'nativeaudio',
        'type': 'static_library',
        'include_dirs': [
            './',
            '../src/',
            '../third-party/portaudio/src/common/',
            '../third-party/zita-resampler/',
            '../third-party/ffmpeg/',
        ],
        'defines': [
            '__STDC_CONSTANT_MACROS'
        ],
        'conditions': [
            ['OS=="win"', {
                'cflags': []
            }, { # OS != "win"
                'cflags': [
                    '-Wall',
                    '-c',
                    '-g',
                    '-O2',
                ],
            }],
        ],
        'sources': [
            'NativeAudioNode.cpp',
            'NativeAudio.cpp',
        ],
    }],
}
