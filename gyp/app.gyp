{
    'targets': [{
        'target_name': 'nativeapp',
        'type': 'executable',
        'dependencies': [
            'network.gyp:nativenetwork',
            'interface.gyp:nativeinterface',
            'av.gyp:nativeav',
            'native.gyp:nativestudio',
        ],
        'conditions': [
            ['OS=="linux"', {
                'cflags': [
                    '-fno-rtti',
                    '-ffunction-sections',
                    '-fdata-sections',
                    '-fno-exceptions',
                    '-DTRIMMED',
                    '-freorder-blocks',
                    '-fomit-frame-pointer'
                ],
                'include_dirs': [
                    '<(native_src_path)',
                    '<(native_network_path)',
                    '<(native_interface_path)',
                    '<(third_party_path)/SDL/include/',
                    '<(third_party_path)/mozilla/js/src/dist/include/',
                ],
                'ldflags': [
                    '-L<(third_party_path)/SDL/build/build/.libs/',
                    '-L<(third_party_path)/skia/out/Release/obj.target/gyp/',
                    '-L<(third_party_path)/mozilla/js/src/',
                    '-L<(third_party_path)/http-parser/',
                    '-L<(third_party_path)/portaudio/lib/.libs/',
                    '-L<(third_party_path)/ffmpeg/libavformat/',
                    '-L<(third_party_path)/ffmpeg/libavcodec/',
                    '-L<(third_party_path)/ffmpeg/libavutil/',
                    '-L<(third_party_path)/ffmpeg/libswscale/',
                    '-L<(third_party_path)/ffmpeg/libswresample/',
                    '-L<(third_party_path)/c-ares/.libs/',
                    '-L<(third_party_path)/libzip/lib/.libs/',
                    '-L<(third_party_path)/zita-resampler/libs/',
                    #-L ../../third-party/angle/out/Debug/obj.target/src/ \
                ],
                'libraries': [
#'-ltranslator_glsl',
#'-ltranslator_common',
#'-lpreprocessor',
                    '-Wl,--start-group',
                    '-lGL',
                    '-lasound',
                    '-lpthread',
                    '-lpng',
                    '-lfreetype',
                    '-lrt',
                    '-lz',
                    '-ldl',

                    '-Wl,-Bstatic '
                    '-lSDL2',
                    '-lzip',
                    '-lcares',
                    '-lhttp_parser',
                    '-ljsoncpp',
                    '-lportaudio',
                    '-lzita-resampler',
                    '-lnspr4',
                    '-Wl,-Bdynamic',

                    '-lswresample',
                    '-lswscale',
                    '-lavformat',
                    '-lavcodec',
                    '-lavutil',
                    '-lskia_sfnt',
                    '-lzlib',
                    '-ljpeg',
                    '-lskia_opts_ssse3',
                    '-lskia_opts',
                    '-lskia_utils',
                    '-lpicture_utils',
                    '-lskia_ports',
                    '-lskia_images',
                    '-lskia_skgr',
                    '-lskia_gr',
                    '-lskia_effects',
                    '-lskia_core',
                    '-ljs_static',
                    '-Wl,--end-group'
                ],
                'sources': [
                    '<(native_interface_path)/linux/main.cpp',
                    '<(third_party_path)/portaudio/src/common/pa_ringbuffer.o'
                ]
            }]
        ],
    }],
}
