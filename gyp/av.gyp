{
    'targets': [{
        'target_name': 'nativeav',
        'type': 'static_library',
        'include_dirs': [
            '<(native_src_path)',
            '<(third_party_path)/portaudio/src/common/',
            '<(third_party_path)/portaudio/include/',
            '<(third_party_path)/zita-resampler/libs/',
            '<(third_party_path)/ffmpeg/',
            '<(third_party_path)/libcoroutine/source/',
            '<(third_party_path)/basekit/source/',
        ],
        'dependencies': [
            '<(native_network_path)/gyp/network.gyp:nativenetwork-includes',
            '<(native_nativejscore_path)/gyp/nativejscore.gyp:nativejscore-includes',
        ],
        'defines': [
            '__STDC_CONSTANT_MACROS'
        ],
        'conditions': [
            ['OS=="mac"', {
                 'cflags+': [
                    '-Wno-c++0x-extensions',
                    '-std=c++11',
                  ]
                }
            ],
            ['OS=="linux"', {
                'cflags+': [
                    '-Wno-c++0x-extensions',
                    '-std=c++11',
                  ]
                }
            ],
        ],
        'sources': [
            '<(native_av_path)NativeAV.cpp',
            '<(native_av_path)NativeAudioNode.cpp',
            '<(native_av_path)NativeAudio.cpp',
            '<(native_av_path)NativeVideo.cpp',
            '<(native_av_path)NativeAudioNodeGain.cpp',
            '<(native_av_path)NativeAudioNodeDelay.cpp',
        ],
    }],
}
