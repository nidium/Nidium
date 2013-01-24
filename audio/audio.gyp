{
    'targets': [{
        'target_name': 'nativeaudio',
        'type': 'static_library',
        'include_dirs': [
            '../src/',
            '../third-party/portaudio/src/common/',
            '../third-party/portaudio/include/',
            '../third-party/zita-resampler/libs/',
            '../third-party/ffmpeg/',
        ],
        'defines': [
            '__STDC_CONSTANT_MACROS'
        ],
        'conditions': [
            [ 
			'OS=="mac"', {
				'xcode_settings': {
					'ARCHS': [
						'x86_64',
					],
					'OTHER_CFLAGS': [
						'-v',
                        '-std=c++11',
                        '-stdlib=libc++'
					],
				},
			}
            ],
        ],
        'sources': [
            'NativeAudioNode.cpp',
            'NativeAudio.cpp',
        ],
    }],
}
