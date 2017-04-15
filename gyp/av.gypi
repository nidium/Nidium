# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'include_dirs': [
        '<(nidium_src_path)',
        '<(third_party_path)/portaudio/src/common/',
        '<(third_party_path)/portaudio/include/',
        '<(third_party_path)/zita-resampler/libs/',
        '<(third_party_path)/ffmpeg/',
        '<(third_party_path)/libcoroutine/source/',
        '<(third_party_path)/basekit/source/',
    ],
    'sources': [
        '<(nidium_av_path)AV.cpp',
        '<(nidium_av_path)AudioNode.cpp',
        '<(nidium_av_path)Audio.cpp',
        '<(nidium_av_path)Video.cpp',
        '<(nidium_av_path)AudioNodeGain.cpp',
        '<(nidium_av_path)AudioNodeDelay.cpp',
        '<(third_party_path)/portaudio/src/common/pa_ringbuffer.o',
    ],
    'direct_dependent_settings': {
        'conditions': [
            ['OS=="win"', {
                "link_settings": {
                    'libraries': [
#                        '$(SDKROOT)/System/Library/Frameworks/CoreAudio.framework',
#                        '$(SDKROOT)/System/Library/Frameworks/CoreVideo.framework',
#                        '$(SDKROOT)/System/Library/Frameworks/VideoDecodeAcceleration.framework',
#                        '$(SDKROOT)/System/Library/Frameworks/CoreAudioKit.framework',
#                        '$(SDKROOT)/System/Library/Frameworks/AudioToolbox.framework',
#                        '$(SDKROOT)/System/Library/Frameworks/AudioUnit.framework',
#                        '$(SDKROOT)/System/Library/Frameworks/CoreMedia.framework',
#                        '$(SDKROOT)/System/Library/Frameworks/VideoToolbox.framework',
#                        '$(SDKROOT)/System/Library/Frameworks/Security.framework',
#                        '-lcoroutine',
                        'portaudio_static_x86.lib', #fixme
                        'libzita-resampler.a',
   #                     'libswresample.a',
#                        'libswscale.a',
   #                     'libavformat.a',
   #                     'libavcodec.a',
   #                     'libavutil.a'
                    ],
                }
            }],
            ['OS=="mac"', {
                "link_settings": {
                    'libraries': [
                        '$(SDKROOT)/System/Library/Frameworks/CoreAudio.framework',
                        '$(SDKROOT)/System/Library/Frameworks/CoreVideo.framework',
                        '$(SDKROOT)/System/Library/Frameworks/VideoDecodeAcceleration.framework',
                        '$(SDKROOT)/System/Library/Frameworks/CoreAudioKit.framework',
                        '$(SDKROOT)/System/Library/Frameworks/AudioToolbox.framework',
                        '$(SDKROOT)/System/Library/Frameworks/AudioUnit.framework',
                        '$(SDKROOT)/System/Library/Frameworks/CoreMedia.framework',
                        '$(SDKROOT)/System/Library/Frameworks/VideoToolbox.framework',
                        '$(SDKROOT)/System/Library/Frameworks/Security.framework',
                        'libcoroutine.a',
                        'libportaudio.a',
                        'libzita-resampler.a',
                        'libswresample.a',
                        'libswscale.a',
                        'libavformat.a',
                        'libavcodec.a',
                        'libavutil.a'
                    ],
                }
            }],
            ['OS=="linux"', {
                "link_settings": {
                    'libraries': [
                        '-lasound',
                        '-lportaudio',
                        '-lzita-resampler',
                        '-lcoroutine',
                        '-lswresample',
                        '-lswscale',
                        '-lavformat',
                        '-lavcodec',
                        '-lavutil',
                    ],
                }
            }]
        ]
    }
}

