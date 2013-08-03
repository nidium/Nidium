{
    'targets': [{
        'target_name': 'nativeav',
        'type': 'static_library',
        'include_dirs': [
            '<(native_src_path)',
            '<(native_network_path)',
            '<(native_nativejscore_path)',
            '<(third_party_path)/portaudio/src/common/',
            '<(third_party_path)/portaudio/include/',
            '<(third_party_path)/zita-resampler/libs/',
            '<(third_party_path)/ffmpeg/',
            '<(third_party_path)/libcoroutine/source/',
            '<(third_party_path)/basekit/source/',
        ],
        'defines': [
            '__STDC_CONSTANT_MACROS'
        ],
        'sources': [
            '<(native_av_path)NativeAV.cpp',
            '<(native_av_path)NativeAudioNode.cpp',
            '<(native_av_path)NativeAudio.cpp',
            '<(native_av_path)NativeVideo.cpp',
        ],
    }],
}
